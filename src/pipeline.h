/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   pipeline.h
 * Author: nikita
 *
 * Created on October 29, 2017, 6:48 PM
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include <gst/gst.h>

#include "uncopyable.h"

class Pipeline : private Uncopyable {
public:
	typedef std::unique_ptr<GstPipeline> pipeline_t;
	typedef std::unique_ptr<GstElement> element_t;
	typedef std::unordered_map<std::string, element_t> elements_map_t;
	typedef std::pair<std::string, std::string> property_t;

private:
	elements_map_t elements;
	pipeline_t pipeline;

public:
	Pipeline();
	virtual ~Pipeline();

	void create_element(std::string& name, std::string& component, std::vector<property_t>& props);
private:

};

#endif /* PIPELINE_H */

