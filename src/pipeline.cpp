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
	gst_init(NULL, NULL);

	this->pipeline = pipeline_t(GST_PIPELINE(gst_pipeline_new("pipeline")), gnu_deleter());
	this->loop = g_main_loop_new(NULL, FALSE);

	this->relation_index = 0;

	auto bus = std::shared_ptr<GstBus>(gst_pipeline_get_bus(this->pipeline.get()), gnu_deleter());
	gst_bus_add_watch(bus.get(), (GstBusFunc) Pipeline::bus_callback, this);

	LOG(INFO) << "Pipeline created";
}

Pipeline::~Pipeline()
{
}

void Pipeline::create_element(std::string name, std::string component, std::vector<property_t> props)
{
	LOG(INFO) << "Pipeline element " << name << ":" << component << " is being created";
	auto element = gst_element_factory_make(component.c_str(), name.c_str());

	g_assert(element);

	for (auto &prop : props) {
		LOG(INFO) << "Setting property " << prop.first << "=" << prop.second;
		g_object_set(G_OBJECT(element), prop.first.c_str(), prop.second.c_str(), NULL);
	}

	this->elements.insert({name, element});
	gst_bin_add(GST_BIN(this->pipeline.get()), element);
}

void Pipeline::create_relation(std::string first_name, std::string second_name, RelationType relation_type, std::vector<std::string> props)
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

		if (!gst_element_link(GST_ELEMENT(first_element), GST_ELEMENT(second_element))) {
			LOG(ERROR) << "Could not initialize relation " << first_name << "→" << second_name << ", can not link elements";
			return;
		}

		this->relations.insert({this->relation_index, std::make_tuple(std::string(), first->second, std::string(), second->second)});

		break;
	}
	case RelationType::Pad:
	{
		if (props.size() != 2) {
			LOG(ERROR) << "Could not initialize relation " << first_name << "→" << second_name << ", incorrect properties";
			return;
		}

		auto first_element = first->second;
		auto second_element = second->second;
		auto src = props[0];
		auto sink = props[1];

		auto result = this->relations.insert({this->relation_index, std::make_tuple(src, first_element, sink, second_element)});

		auto *relation = &(result.first->second);

		auto handler_id = g_signal_connect(first_element, "pad-added", G_CALLBACK(&Pipeline::on_pad_added), relation);

		if (handler_id == 0) {
			LOG(ERROR) << "Could not initialize relation " << first_name << "→" << second_name << ", can not add callback";
			return;
		}

		break;
	}
	}

	LOG(INFO) << "Relation " << first_name << "→" << second_name << " created";

	return;
}

void Pipeline::set_state(GstState state)
{
	auto state_ret = gst_element_set_state(GST_ELEMENT(this->pipeline.get()), state);

	if (state_ret == GstStateChangeReturn::GST_STATE_CHANGE_FAILURE) {
		LOG(ERROR) << "Unable to set state to " << state;
	}
}

void Pipeline::run()
{
	g_main_loop_run(this->loop);
}

Pipeline::MediaInfo Pipeline::media_get_info()
{
	gint64 pos, len;

	if (gst_element_query_position(GST_ELEMENT(this->pipeline.get()), GST_FORMAT_TIME, &pos)
		&& gst_element_query_duration(GST_ELEMENT(this->pipeline.get()), GST_FORMAT_TIME, &len)) {
		
		LOG(DEBUG) << "Pipeline information " << pos << "/" << len;
		return {pos, len};
	}

	LOG(ERROR) << "Can not get media info";
	return {0, 0};
}

void Pipeline::media_rewind(gint64 new_position)
{
	if (gst_element_seek(GST_ELEMENT(this->pipeline.get()), 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, new_position, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {

		LOG(DEBUG) << "Pipeline has been rewinded to " << new_position;
		return;
	}

	LOG(ERROR) << "Rewind failed!";
}

void Pipeline::on_pad_added(GstElement* element, GstPad* first_pad, relation_t* relation)
{
	auto caps = gst_pad_get_current_caps(first_pad);
	auto structure = gst_caps_get_structure(caps, 0);
	auto name = (gchar*) gst_structure_get_name(structure);

	LOG(DEBUG) << "Pad added " << name;

	auto src = std::get<0>(*relation);
	auto sink = std::get<2>(*relation);
	auto target = std::get<3>(*relation);

	LOG(DEBUG) << "Check pad relation " << src << "→" << sink;

	if (g_strrstr(name, src.c_str())) {
		auto second_pad = std::shared_ptr<GstPad>(gst_element_get_static_pad(target, sink.c_str()), gnu_deleter());

		if (!second_pad) {
			LOG(ERROR) << "Could not initialize relation " << name << "→" << sink << ", can not find sink";
			return;
		}

		auto result = gst_pad_link(first_pad, second_pad.get());

		if (result != GstPadLinkReturn::GST_PAD_LINK_OK) {
			LOG(ERROR) << "Could not initialize relation " << name << "→" << sink << ", pad_link returned " << result;
			return;
		}

		LOG(DEBUG) << "Pad relation initiated " << name << "→" << sink;
		return;
	}

	LOG(DEBUG) << "Ignore relation " << name << "→" << sink;
}

gboolean Pipeline::bus_callback(GstBus* bus, GstMessage* message, Pipeline* pipeline)
{
	switch (GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR:
	{
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		LOG(ERROR) << err->message;

		g_error_free(err);
		g_free(debug);
		g_main_loop_quit(pipeline->loop);
		break;
	}

	case GST_MESSAGE_EOS:
	{
		LOG(INFO) << "End of stream";
		g_main_loop_quit(pipeline->loop);
		break;
	}
	default:
		break;
	}

	return TRUE;
}
