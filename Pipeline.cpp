#include "Pipeline.h"
#include "Model.h"

// std
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace engine {

	Pipeline::Pipeline(const std::string& vertFilepath, 
		const std::string& fragFilepath, 
		Device& tempDevice, 
		const PipelineConfigInfo& configInfo) : device(tempDevice) {

		createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
	}

	Pipeline::~Pipeline() {
		vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
	}

	std::vector<char> Pipeline::readFile(const std::string& filepath) {
		
		std::ifstream file{ filepath, std::ios::ate | std::ios::binary };	//Open the file using the file path passed in
																			//ios ate to go to the end of the file
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filepath);
		}
		size_t fileSize = static_cast<size_t>(file.tellg());				//tellg gets the last position which is also the file size

		std::vector<char> buffer(fileSize);									//Initialize this vector with the file size

		file.seekg(0);								//Seek to the start of the file
		file.read(buffer.data(), fileSize);			//Read the file into the buffer vector with the correct number of bytes.
		file.close();								//Close the file
		return buffer;								//Return the vector
	}

	// Here we read the SimpleShader files (.vert and .frag) but, we actually read the the files 
	// that have the .spv extension. These can be found in the Shader folder in the main project 
	// folder. Those files are a result of the SimpleShader files being compiled by the compile.bat
	void Pipeline::createGraphicsPipeline(
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo) {

		assert(
			configInfo.pipelineLayout != VK_NULL_HANDLE && 
			"Cannot create graphics pipeline: No pipeline layout provided in configInfo");

		assert(
			configInfo.renderPass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: No render pass provided in configInfo");

		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilepath);

		//Let's initialize our shader modules
		createShaderModule(vertCode, &vertShaderModule);
		createShaderModule(fragCode, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];

		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;	//This says that this stage is for the vertex shader
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";						//This is the name of our entry function in the shader
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		//This one is a mechanism to customize shader functionality
		shaderStages[0].pSpecializationInfo = nullptr;

		//Same as above except for the fragment shader instead of the vertex
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// We get our vertex descriptions from our model class to create the struct below this
		auto& bindingDescriptions = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;
		
		// This struct is used to describe how we interpret our vertex 
		// buffer data that is the initial data into our graphics pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;						// 2 stages, fragment and vertex shaders
		pipelineInfo.pStages = shaderStages;				// Now we pass in the shaderStages that we set above
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;

		//This is an optional setting used to dynamically configure some pipeline functionalities 
		//such as line width or the viewport without needing to recreate the pipeline.
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		
		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		//These can be used when trying to optimize performance. It can be less expensive 
		//for a GPU to create a new graphics pipeline by deriving from an existing one.
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		//Now we create the graphics pipeline using all the info and check whether it worked or not.
		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline");
		}
	}

	void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module");
		}
	}

	// The VK_PIPELINE_BIND_POINT_GRAPHICS option signals that this is a 
	// graphics pipeline and not a compute pipeline or a ray tracing pipeline
	// There is no need to validate the graphics pipeline because it must have
	// been properly created at initialization in order for us to have gotten here
	void Pipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	// This is the first step in the graphics pipeline called the Input Assembler. 
	// It takes a collection of vertices and turns them into geometry.
	void Pipeline::defultPipelineConfigInfo(PipelineConfigInfo& configInfo) {

		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		
		//This tells Vulkan that we want a traingle made from the vertices and not a line or anything else
		//It also tells it to make a list so that every three vertices gets collected into their own triangles
		//Traingle strip is also possible, it saves memory but limits geometry.
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		//The viewport describes the transfomation between our pipeline's output and our target image
		//In the SimpleShader.vert, we used a gl variable that takes coordinates in range -1 to 1. But
		//images are represented by pixels which are from 0,0 to the width,height. This viewport tells 
		//our pipeline how we want to transform our gl_position values to the output output image.
		//It dictates the width and height of the image. 
		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		
		//This is like the viewport, but instead of dictating the width and height of the image, it sets the bounds and
		//cuts off whatever is not naturally inside of the scissor rectangle.
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		// This stage breakes up our geometry into fragments, for each pixel our triangle overlaps
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;			//This forces the Z component of gl_position to be between 0 and 1
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;	//Discards all primitives before rasterization, we make it false.
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;	//Describes how to draw the triangles, whether just the vertices, lines or fill
		configInfo.rasterizationInfo.lineWidth = 1.0f;						//Bruh

		//Here we can use culling to discard traingles that are backward facing. It knows based on which vertex is inputted first
		//and does sort of a walk around the triangle to see it it's facing the correct way aka the winding order. There are significant 
		//benefits to culling backward facing traingles. But since this is a default setting, we keep it to none. The front face will be
		//determined by the direction we tell it to go. Here we make it clockwise by default so that we don't accidentally cull our first 
		//traingle. Any traingle who's vertices don't match the desired clockwise direction will be identified as backwards.
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

		//Can be used to alter depth values by addding a constant value or by factor of the fragment's slope
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		//This relates to how the rasterizer handles the edges of geometry so that fragments can be partially in a triangle. Without this,
		//the fragment is either fully in or fully out of a triangle depending on where the center of the pixel is. This can lead to ugly
		//artifacs such as aliasing which makes the objects looks jagged. MSAA (multisample anti-aliasing) takes multiple samples are taken
		//along the edges of geometry to better approximate how much of the fragment is contained by the traingle, and using that information
		//to shade the pixel by a variable amount which then leads to a higher quality image.
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
		configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		//This is controls how we combine colors in our frame buffer. If we have overlapping traingles, our fragment shader will return multiple
		//colors for some pixels in our frame buffer.
		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;							//We can optionally enable color blending
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;		// Optional
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;					// Optional
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;		// Optional
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;					// Optional

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		//This is depth testing. It contains the configuration for how each fragmane is layered. It doesn't contain layer info for
		//the whole image but rather each pixel. Pixels that are "behind" other pixels are discarded based on their depth value.
		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};  // Optional
		configInfo.depthStencilInfo.back = {};   // Optional
		
		// This configures the pipeline to expect a dynamic viewport and dynamic scissor to be provided later
		configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.flags = 0;

		configInfo.bindingDescriptions = Model::Vertex::getBindingDescriptions();
		configInfo.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
	}
}