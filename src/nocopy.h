/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nocopy.h
 * Author: nikita
 *
 * Created on October 29, 2017, 8:06 PM
 */

#ifndef NOCOPY_H
#define NOCOPY_H

class Uncopyable {
public:

	Uncopyable()
	{
	}

private:
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);
};


#endif /* NOCOPY_H */

