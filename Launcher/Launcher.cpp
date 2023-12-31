#pragma once
#include<iostream>
#include<Log.h>
#include<RigidBody.h>

using namespace std;

int main()
{
	cout << "Hello Launcher" << endl;
	Log::warn("Hello");
	Log::info("Hello");
	Log::success("Hello");
	Log::error("Hello");
	rigidbody::show();
	return 0;
}
