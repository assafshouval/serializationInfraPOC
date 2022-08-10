// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "Archive.h"
#include <cassert>


int main() 
{
	std::vector<std::shared_ptr<Base>> objects; 
	objects.push_back(std::make_shared<Point>(1, 2)); 
	objects.push_back(std::make_shared<Point>(50, 12)); 
	objects.push_back(std::make_shared<Circle>(0, 0, 10)); 
	{
		OutArchive write_archive("out.dat"); 
		write_archive.write(objects); 
	}
	std::vector<std::shared_ptr<Base>> loaded_objects; 
	InArchive read_archive("out.dat");
	read_archive.read(loaded_objects); //loaded_objects should contain the same objects as in objects
#ifdef _DEBUG
	for (size_t i = 0; i < objects.size(); i++)
	{
		int res = std::memcmp(objects[i].get(), loaded_objects[i].get(), sizeof(objects[i].get()) == 0);
		assert(res==0);
	}
#endif	

	
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
