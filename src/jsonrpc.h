/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   jsonrpc.h
 * Author: nikita
 *
 * Created on October 29, 2017, 8:01 PM
 */

#ifndef JSONRPC_H
#define JSONRPC_H

#include <string>
#include <memory>
#include <unordered_map>

#include <json/json.h>
#include <json/reader.h>

#include "uncopyable.h"

class JsonRpc : Uncopyable {
public:
	constexpr static char JSONRPC_VER[] = "jsonrpc";
	constexpr static char JSONRPC_VER_20[] = "2.0";
	constexpr static char JSONRPC_METHOD[] = "method";
	constexpr static char JSONRPC_ID[] = "id";
	constexpr static char JSONRPC_PARAMS[] = "params";

	class Callee {
	public:
		virtual bool validate(Json::Value params) = 0;
		virtual void call(Json::Value params) = 0;
	};

	typedef std::shared_ptr<JsonRpc::Callee> handler_t;
	typedef std::unordered_map<std::string, handler_t> handlers_t;

private:
	handlers_t handlers;

protected:
	void try_execute(std::string &data);

public:
	JsonRpc();
	virtual ~JsonRpc();

	void add_handler(std::string method, handler_t handler_ptr);
};

#endif /* JSONRPC_H */

