#include "findedges.h"

namespace inviwo {

FindEdges::FindEdges()
    : ProcessorGL(),
      inport0_(Port::INPORT, "inport0"),
      outport_(Port::OUTPORT, "outport"),
      alpha_("alpha", "Alpha", 0.5f, 0.0f, 1.0f)
{
    addPort(inport0_);
    addPort(outport_);
    addProperty(alpha_);
}

FindEdges::~FindEdges() {}

Processor* FindEdges::create() const {
    return new FindEdges();
}

void FindEdges::initialize() {
    ProcessorGL::initialize();
    shader_ = new Shader("img_findedges.frag");
}

void FindEdges::deinitialize() {
    delete shader_;
    Processor::deinitialize();
}

void FindEdges::process() {    
    
    Image* outImage = outport_.getData();
    ImageGL* outImageGL = outImage->getRepresentation<ImageGL>();
    ivec2 csize = outImageGL->dimension();

    activateTarget(outport_);
    bindColorTexture(inport0_, GL_TEXTURE0);

    shader_->activate();
    shader_->setUniform("inport0_", 0);
    shader_->setUniform("alpha_", alpha_.get());
    shader_->setUniform("dimension_", vec2(1.f / csize[0], 1.f / csize[1]) );
    renderImagePlaneQuad();
    shader_->deactivate();

    deactivateCurrentTarget();
}

} // namespace
