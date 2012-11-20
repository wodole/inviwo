#include <QPainter>

#include <QPainterPath>

#include "inviwo/qt/editor/connectiongraphicsitem.h"

namespace inviwo {

CurveGraphicsItem::CurveGraphicsItem(QPointF startPoint, QPointF endPoint, bool layoutOption, ivec3 color, bool dragMode)
                                     : startPoint_(startPoint),
                                       endPoint_(endPoint),
                                       verticalLayout_(layoutOption),
                                       color_(color.r, color.g, color.b),
                                       dragMode_(dragMode){
    setZValue(CONNECTIONGRAPHICSITEM_DEPTH);
}

CurveGraphicsItem::~CurveGraphicsItem() {}

QPainterPath CurveGraphicsItem::obtainCurvePath() const {
    float delta = endPoint_.y()-startPoint_.y();
    
    QPointF ctrlPoint1 = QPointF(startPoint_.x(), endPoint_.y()-delta/4.0);
    QPointF ctrlPoint2 = QPointF(endPoint_.x(), startPoint_.y()+delta/4.0);

    if (!verticalLayout_) {
        delta = endPoint_.x()-startPoint_.x();
        ctrlPoint1 = QPointF(startPoint_.x()+delta/4.0, startPoint_.y());
        ctrlPoint2 = QPointF(endPoint_.x()-delta/4.0, endPoint_.y());
    }

    QPainterPath bezierCurve;
    bezierCurve.moveTo(startPoint_);
    bezierCurve.cubicTo(ctrlPoint1, ctrlPoint2, endPoint_);
    return bezierCurve;
}

void CurveGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);

    if(!dragMode_) {
        p->setPen(QPen(Qt::black, 3.5, Qt::SolidLine, Qt::RoundCap));
        p->drawPath(obtainCurvePath());
    }

    if (isSelected()) p->setPen(QPen(Qt::darkRed, 2.0, Qt::SolidLine, Qt::RoundCap));
    else p->setPen(QPen(color_, 2.0, Qt::SolidLine, Qt::RoundCap));
    p->drawPath(obtainCurvePath());
}

QPainterPath CurveGraphicsItem::shape() const {
    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    return pathStrocker.createStroke(obtainCurvePath());
}

QRectF CurveGraphicsItem::boundingRect() const {
    QPointF topLeft = QPointF(std::min(startPoint_.x(), endPoint_.x()),
                              std::min(startPoint_.y(), endPoint_.y()));
    return QRectF(topLeft.x()-30.0, topLeft.y()-30.0,
                  abs(startPoint_.x()-endPoint_.x())+60.0,
                  abs(startPoint_.y()-endPoint_.y())+60.0);
}



ConnectionGraphicsItem::ConnectionGraphicsItem(ProcessorGraphicsItem* outProcessor, Port* outport,
                                               ProcessorGraphicsItem* inProcessor, Port* inport, bool layoutOption)
                                               : CurveGraphicsItem(outProcessor->mapToScene(outProcessor->calculatePortRect(outport)).boundingRect().center(),
                                                                   inProcessor->mapToScene(inProcessor->calculatePortRect(inport)).boundingRect().center(), 
                                                                   layoutOption, inport->getColorCode(), false),
                                                 outProcessor_(outProcessor), outport_(outport),
                                                 inProcessor_(inProcessor), inport_(inport) {
    setFlags(ItemIsSelectable | ItemIsFocusable);
}

ConnectionGraphicsItem::~ConnectionGraphicsItem() {}

void ConnectionGraphicsItem::flipLayout() {
    setStartPoint(outProcessor_->mapToScene(outProcessor_->calculatePortRect(outport_)).boundingRect().center());
    setEndPoint(inProcessor_->mapToScene(inProcessor_->calculatePortRect(inport_)).boundingRect().center());

    if (getLayoutOption()) {
        setLayoutOption(false);
    }
    else
        setLayoutOption(true);

}

} // namespace