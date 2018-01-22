/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/animationqt/animationeditorqt.h>
#include <modules/animationqt/trackqt.h>
#include <modules/animationqt/keyframeqt.h>
#include <modules/animationqt/keyframesequenceqt.h>
#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>

#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

namespace animation {

AnimationEditorQt::AnimationEditorQt(AnimationController& controller)
    : QGraphicsScene(), controller_(controller) {
    auto& animation = *controller_.getAnimation();
    animation.addObserver(this);

	// Add Control track
	{
		auto trackQt = std::make_unique<TrackQt>(animation.getControlTrack());
		trackQt->setPos(0, TimelineHeight + TrackHeight * 0.5);
		this->addItem(trackQt.get());
		tracks_.push_back(std::move(trackQt));
	}

	// Add Property tracks
    for (size_t i = 0; i < animation.size(); ++i) {
        auto trackQt = std::make_unique<TrackQt>(animation[i]);
        trackQt->setPos(0, TimelineHeight + TrackHeight * (i + 1) + TrackHeight * 0.5);
        this->addItem(trackQt.get());
        tracks_.push_back(std::move(trackQt));
    }

    // Add drag&drop indicator
    QPen timePen;
    timePen.setColor(QColor(255, 128, 0));
    timePen.setWidthF(1.0);
    timePen.setCosmetic(true);
    timePen.setStyle(Qt::DashLine);
    pDropIndicatorLine = addLine(10, 0, 10, 1000, timePen);
    if (pDropIndicatorLine) {
        pDropIndicatorLine->setZValue(1);
        pDropIndicatorLine->setVisible(false);
    }

    //Add drag&drop hint - this one still has a number of issue. It should probably not be in the scene, not be affected by zoom.
    pDropIndicatorText = new QGraphicsSimpleTextItem(pDropIndicatorLine);
    if (pDropIndicatorText) {
        pDropIndicatorText->setPos(0, TimelineHeight);
        pDropIndicatorText->setZValue(1);
        pDropIndicatorText->setVisible(false);
    }

    updateSceneRect();
}

AnimationEditorQt::~AnimationEditorQt() = default;

void AnimationEditorQt::onTrackAdded(Track* track) {
    auto trackQt = std::make_unique<TrackQt>(*track);
    trackQt->setPos(0, TimelineHeight + TrackHeight * tracks_.size() + TrackHeight * 0.5);
    this->addItem(trackQt.get());
    tracks_.push_back(std::move(trackQt));
    updateSceneRect();
}

void AnimationEditorQt::onTrackRemoved(Track* track) {
    if (util::erase_remove_if(tracks_, [&](auto& trackqt) {
            if (&(trackqt->getTrack()) == track) {
                this->removeItem(trackqt.get());
                return true;
            } else {
                return false;
            }
        }) > 0) {

        for (size_t i = 0; i < tracks_.size(); ++i) {
            tracks_[i]->setY(TimelineHeight + TrackHeight * i + TrackHeight * 0.5);
        }
    }
    updateSceneRect();
}

void AnimationEditorQt::keyPressEvent(QKeyEvent* keyEvent) {
    int k = keyEvent->key();
    if (k == Qt::Key_Delete) {  // Delete selected
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            if (auto keyqt = qgraphicsitem_cast<KeyframeQt*>(elem)) {
                auto& animation = *controller_.getAnimation();
                animation.removeKeyframe(&(keyqt->getKeyframe()));
            }
			else if (auto seqqt = qgraphicsitem_cast<KeyframeSequenceQt*>(elem)) {
				auto& animation = *controller_.getAnimation();
				animation.removeKeyframeSequence(&(seqqt->getKeyframeSequence()));
			}
        }
    } else if (k == Qt::Key_Space) {
        switch(controller_.getState()) {
            case AnimationState::Paused:
                controller_.play();
                break;
            case AnimationState::Playing:
                controller_.pause();
                break;
        } 
    }
}

void AnimationEditorQt::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    // For now only accept PropertyWidget
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    event->setAccepted(source != nullptr && source->getProperty() != nullptr);
}

void AnimationEditorQt::dragLeaveEvent(QGraphicsSceneDragDropEvent * event) {
    if (pDropIndicatorLine) pDropIndicatorLine->setVisible(false);
}

void AnimationEditorQt::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    // Must override for drop events to occur. Do not call QGraphicsScene::dragMoveEvent

    //Indicate position
    if (pDropIndicatorLine) {
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX = getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        pDropIndicatorLine->setLine(snapX, 0, snapX, pView ? pView->height() : height());
        pDropIndicatorLine->setVisible(true);
    }

    //Indicate insertion mode: keyframe or keyframe sequence.
    if (pDropIndicatorText) {
        QString Text = (event->modifiers() & Qt::ControlModifier)
                        ? "Insert new keyframe sequence (Alt for non-snapping time)"
                        : "Insert new keyframe (Ctrl for sequence, Alt for non-snapping time)";

        pDropIndicatorText->setText(Text);
        pDropIndicatorText->setVisible(true);
    }


    event->accept();
}
    
void AnimationEditorQt::dropEvent(QGraphicsSceneDragDropEvent *event) {

    //Switch off drag&drop indicator
    if (pDropIndicatorLine) pDropIndicatorLine->setVisible(false);

    //Drop it into the scene
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    if (source) {
        auto property = source->getProperty();
        auto app = controller_.getInviwoApplication();

        //Get time
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX = getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        const qreal time = snapX / static_cast<double>(WidthPerSecond);

        // Use AnimationManager for adding keyframe or keyframe sequence.
        auto& am = app->template getModuleByType<AnimationModule>()->getAnimationManager();
        if (event->modifiers() & Qt::ControlModifier) {
            am.addSequenceCallback(property, Seconds(time));
        } else {
            am.addKeyframeCallback(property, Seconds(time));
        }

        //Thanks
        event->acceptProposedAction();
    }
}

void AnimationEditorQt::updateSceneRect() {
    setSceneRect(0.0, 0.0, controller_.getAnimation()->lastTime().count() * WidthPerSecond,
                 controller_.getAnimation()->size() * TrackHeight + TimelineHeight);
}

void AnimationEditorQt::onFirstMoved() { updateSceneRect(); }

void AnimationEditorQt::onLastMoved() { updateSceneRect(); }

}  // namespace

}  // namespace
