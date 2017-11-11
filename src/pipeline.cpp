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
	this->relation_index = 0;

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

void Pipeline::create_relation(std::string& first_name, std::string& second_name, RelationType relation_type, std::vector<std::string>& props)
{
	auto first = this->elements.find(first_name);
	auto second = this->elements.find(second_name);
	auto not_found = this->elements.end();

	if (first == not_found || second == not_found) {
		LOG(ERROR) << "Could not initialize relation " << first_name << "→" << second_name << ", some of them is not found";
		return;
	}

	this->relation_index++;

	switch (relation_type) {
	case RelationType::Simple:
	{

		auto first_element = first->second;
		auto second_element = second->second;

		if (!gst_element_link(GST_ELEMENT(first_element.get()), GST_ELEMENT(second_element.get()))) {
			LOG(ERROR) << "Could not initialize relation " << first_name << "→" << second_name << ", can not link elements";
			return;
		}

		this->relations.insert({this->relation_index, std::make_tuple(std::string(), first->second, std::string(), second->second)});

		break;
	}
	}
	return;
}

