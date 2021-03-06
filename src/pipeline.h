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
#include <tuple>

#include <gst/gst.h>

#include "uncopyable.h"
#include "gnu_deleter.h"

class Pipeline : private Uncopyable {
public:

	typedef GstElement* element_t;
	typedef GMainLoop* loop_t;
	typedef std::shared_ptr<GstPipeline> pipeline_t;
	typedef std::tuple<std::string, element_t, std::string, element_t> relation_t;
	typedef std::pair<std::string, std::string> property_t;
	typedef std::unordered_map<std::string, element_t> elements_map_t;
	typedef std::unordered_map<uint32_t, relation_t> relations_t;

	enum class RelationType {
		Simple = 0,
		Pad = 1,
	};

	struct MediaInfo {
		gint64 position;
		gint64 length;
	};

private:
	elements_map_t elements;
	loop_t loop;
	pipeline_t pipeline;
	relations_t relations;
	uint32_t relation_index;

public:
	Pipeline();
	virtual ~Pipeline();

	void create_element(std::string name, std::string component, std::vector<property_t> props = {});
	void create_relation(std::string first, std::string second, RelationType relation_type, std::vector<std::string> props = {});
	void set_state(GstState state);
	void run();

	MediaInfo media_get_info();
	void media_rewind(gint64 new_position);
private:
	void static on_pad_added(GstElement* element, GstPad* pad, relation_t* relation);
	gboolean static bus_callback(GstBus *bus, GstMessage *message, Pipeline *pipeline);

};

#endif /* PIPELINE_H */

