/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2013 Inviwo Foundation
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
 * Main file author: Peter Steneteg
 *
 *********************************************************************************/

#include <modules/<lname>/<lname>module.h>

namespace inviwo {

<name>Module::<name>Module() : InviwoModule() {
    setIdentifier("<name>");
	
	// Add a directory to the search path of the Shadermanager
	//ShaderManager::getPtr()->addShaderSearchPath(InviwoApplication::PATH_MODULES, "<lname>/glsl");
	
	// Register objects that can be shared with the rest of inviwo here:
	
	// Processors
	// registerProcessor(<name>);
	
	// Properties
	// registerProperty(<name>Property);
	
	// Readers and writes
	// registerDataReader(new <name>Reader());
	// registerDataWriter(new <name>Writer());
	
	// Data converters
	// registerRepresentationConverter(new <name>Disk2RAMConverter());

	// Ports
	// registerPort(<name>Outport);
	// registerPort(<name>Inport);

	// PropertyWidgets
	// registerPropertyWidget(<name>PropertyWidgetQt, <name>Property, "Default");
	
	// Dialogs
	// registerDialog("<lname>", <name>DialogQt);
	
	// Other varius things
	// registerCapabilities(Capabilities* info);
    // registerData(Data* data);
    // registerDataRepresentation(DataRepresentation* dataRepresentation);
	// registerSettings(new SystemSettings());
	// registerMetaData(MetaData* meta);   
    // registerPortInspector(PortInspector* portInspector);
    // registerProcessorWidget(std::string processorClassName, ProcessorWidget* processorWidget);
    // registerRenderer(GeometryRenderer* renderer);
    // registerResource(Resource* resource);	
}

} // namespace
