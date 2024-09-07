//*************************************************************
// This file reads the .vert and .frag files that are compiled 
// by Vulkan for the graphics card and uses them for our engine
// This class is responsible for determining the lifetime of
// the shader code resources and vulkan shader resources
//*************************************************************

#pragma once

#include "Device.h"

#include <string>
#include <vector>

namespace engine {

	//This contains the data sepcifying how we want to configure our pipeline. This info
	//will be accessible by our application and is easily shared among the rest of the program.
	struct PipelineConfigInfo {
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;				//More clean up so that we don't accidentally 
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;	//duplicate the pointers to our Vulkan objects
		PipelineConfigInfo() = default;
		
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;			//These are set in the defultPipelineConfigInfo function
		VkPipelineColorBlendAttachmentState colorBlendAttachment;		//Thse are given default values
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;

		//These ones are not given default values but are set outside of the function
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline {
	private:
		Device& device;						//This stores our device reference
		VkPipeline graphicsPipeline;		//This is the handle to our Vulkan pipeline object
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		static std::vector<char> readFile(const std::string& vertFilepath);

		void createGraphicsPipeline(const std::string& vertFilepath, 
			const std::string& fragFilepath, 
			const PipelineConfigInfo& info);

		//This takes in the shader code in the form of a vector of characters and a pointer to a shader module
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	public:
		Pipeline(const std::string& vertFilepath, 
			const std::string& fragFilepath, 
			Device& tempDevice, 
			const PipelineConfigInfo &info);

		~Pipeline();
		Pipeline(const Pipeline&) = delete;				//More clean up so that we don't accidentally 
		Pipeline &operator=(const Pipeline&) = delete;	//duplicate the pointers to our Vulkan objects

		void bind(VkCommandBuffer commandBuffer);

		static void defultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	};
}