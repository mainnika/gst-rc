/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   jsonrpc.cpp
 * Author: nikita
 * 
 * Created on October 29, 2017, 8:01 PM
 */


#include "jsonrpc.h"

#include "easylogging++.h"

JsonRpc::JsonRpc()
{
}

JsonRpc::~JsonRpc()
{
}

void JsonRpc::try_execute(std::string& data)
{
	Json::Value root;
	Json::CharReaderBuilder reader_builder;

	std::shared_ptr<Json::CharReader> reader(reader_builder.newCharReader());
	JSONCPP_STRING errs;

	if (!reader->parse(data.c_str(), data.c_str() + data.length(), &root, &errs)) {
		LOG(WARNING) << "Not a jsonrpc query:";
		LOG(WARNING) << data;
		return;
	}

	auto ver = root.get(JsonRpc::JSONRPC_VER, Json::Value::null);
	auto method = root.get(JsonRpc::JSONRPC_METHOD, Json::Value::null);
	auto id = root.get(JsonRpc::JSONRPC_ID, Json::Value::null);
	auto params = root.get(JsonRpc::JSONRPC_PARAMS, Json::Value::null);

	auto is_valid = ver.isString() && method.isString()
		&& ver.asString() == JsonRpc::JSONRPC_VER_20;

	if (!is_valid) {
		LOG(WARNING) << "Not a jsonrpc query: " << data;
		return;
	}

	LOG(INFO) << "Query: "
		<< "\n\t" << ver
		<< "\n\t" << method
		<< "\n\t" << id;

	auto callee = this->handlers.find(method.asString());

	if (callee == this->handlers.end()) {
		LOG(WARNING) << "No method " << method.asString() << " availabe";
		return;
	}

	try {
		if (!callee->second->validate(params)) {
			LOG(WARNING) << "Params of method " << method.asString() << " have not passed validation";
			return;
		}

		callee->second->call(params);
	} catch (...) {
		LOG(WARNING) << "Method " << method.asString() << " failed";
	}
}

void JsonRpc::add_handler(std::string method, std::shared_ptr<JsonRpc::Callee> handler_ptr)
{
	auto inserted = this->handlers.insert({method, handler_ptr});

	if (inserted.second) {
		LOG(ERROR) << "Handler " << method << " already exist";
		return;
	}
}

constexpr char JsonRpc::JSONRPC_VER[];
constexpr char JsonRpc::JSONRPC_VER_20[];
constexpr char JsonRpc::JSONRPC_METHOD[];
constexpr char JsonRpc::JSONRPC_ID[];
constexpr char JsonRpc::JSONRPC_PARAMS[];
