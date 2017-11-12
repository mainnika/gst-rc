/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gnu_deleter.h
 * Author: nikita
 *
 * Created on November 12, 2017, 3:21 AM
 */

#ifndef GNU_DELETER_H
#define GNU_DELETER_H

#include <gst/gst.h>

#include "easylogging++.h"

struct gnu_deleter {

	void operator()(void* p) const
	{
		LOG(DEBUG) << "Unref" << p;
		gst_object_unref(p);
	}
};


#endif /* GNU_DELETER_H */

