/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   pipeline.cpp
 * Author: nikita
 * 
 * Created on October 29, 2017, 6:48 PM
 */

#include "pipeline.h"

#include "easylogging++.h"

Pipeline::Pipeline()
{
	this->pipeline = pipeline_t(GST_PIPELINE(gst_pipeline_new("pipeline")));

	LOG(INFO) << "Pipeline created";
}

Pipeline::~Pipeline()
{
}

void Pipeline::create_element(std::string& name, std::string& component, std::vector<property_t>& props)
{

	LOG(INFO) << "Pipeline element " << name << ":" << component << " is being created";
	auto element = gst_element_factory_make(name.c_str(), component.c_str());

	g_assert(element);

	for (auto &prop : props) {
		LOG(INFO) << "Setting property " << prop.first << "=" << prop.second;
		g_object_set(G_OBJECT(element), prop.first.c_str(), prop.second.c_str(), NULL);
	}

	this->elements.insert({name, element_t(element)});
	gst_bin_add(GST_BIN(this->pipeline.get()), element);
}
{
}

