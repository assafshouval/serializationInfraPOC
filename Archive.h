#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <iosfwd>
#include <functional>
#include <typeindex>


/*TODO:
*	serialize should be templated or have a different functions ?
*	should use the same class for serialize and deSerialize?
*	Need to add version
*	how to get the type used for serialization
*	Serialize ID maybe should be dictionary or by typeof
*	add validation of chunks
* */

class Base;
//#todo: check that this is one-to-one dictionary, check how to make it compile time error
static std::map<std::type_index, std::string> typesTags =
{
	{std::type_index(typeid(int)), "int"},
	{std::type_index(typeid(float)), "float"},
	{std::type_index(typeid(std::string)), "string"},
	{std::type_index(typeid(std::vector<std::shared_ptr<Base>>)), "ObjectsArray"}
};
//#TODO: serializer shouldn't know about Base
class ISerializer
{
public:
	virtual void serializeInt(int i) = 0;
	virtual void serializeString(std::string s) = 0;
	virtual void serializeFloat(float f) = 0;
	virtual void serializeObj(std::shared_ptr<Base> b) = 0;
	virtual void serializeObjectsArray(std::vector<std::shared_ptr<Base>> objects) = 0;
};

class TextSerializer : public ISerializer
{
public:
	TextSerializer(std::string path);
	virtual ~TextSerializer() 
	{
		ofstreamObj.close(); 
	}
	virtual void serializeInt(int i);
	virtual void serializeString(std::string s);
	virtual void serializeFloat(float f);
	virtual void serializeObj(std::shared_ptr<Base> b);
	virtual void serializeObjectsArray(std::vector<std::shared_ptr<Base>> objects);
private:
	std::ofstream ofstreamObj;
};

class IDeserializer
{
public:
	//basic types:
	virtual int deserializeInt() = 0;
	virtual std::string deserializeString() = 0;
	virtual float deserializeFloat() = 0;
	virtual void deserializeObjectsArray(std::vector<std::shared_ptr<Base>>& objects) = 0;
	
};
class TextDeserializer : public IDeserializer
{
public:	
	TextDeserializer(std::string fName);
	virtual int deserializeInt();
	virtual std::string deserializeString();
	virtual float deserializeFloat();
	
	//#TODO:ad-hock code, should use proper xml lib (with safety and performance)
	virtual void deserializeObjectsArray(std::vector<std::shared_ptr<Base>>& objects);

	//#TODO: don't need this anymore
	void ReadXMLChunk(const std::string& chunkName,const char*& argEnd, const std::string& xml, const char*& argStart, std::vector<std::string>& args);

private:
	template <class T>
	std::string deserializeToString()
	{
		std::string typeChunk = typesTags[std::type_index(typeid(T))];
		it += (size_t(2)/*"<Type>"*/ + typeChunk.length());
		auto leftOffset = std::distance(stringXML.begin(), it);
		auto rightOffset = stringXML.find_first_of("</", leftOffset) - leftOffset;
		std::string s = stringXML.substr(leftOffset, rightOffset);
		it += (s.length() + size_t(3)/*"</Type>"*/ + typeChunk.length());
		return s;
	}
	std::string stringXML;
	std::string::iterator it;
};


class Base
{
private:
	
protected:
	std::string name;
public:
	Base() {}
	virtual std::string serializedId() { return "Base"; }
	virtual void serialize(ISerializer* serial) const;
	static void deserialize(IDeserializer* serial, Base* b);
};



class Point : public Base
{
private:
	float x;
	float y;
public:
	Point() : x(0),y(0) {}
	Point(float _x, float _y);
	/*ISerializable implementation*/
	virtual std::string serializedId() override { return "Point"; }
	virtual void serialize(ISerializer* serial) const override;
	static void deserialize(IDeserializer* serial, Base* b);

};

class Circle : public Point
{
private:
	int r;
public:
	Circle() : r(0) {}
	Circle(float _x, float _y, int _r) : Point(_x, _y), r(_r) {}
	
	/*ISerializable implementation*/
	virtual std::string serializedId()
	{
		return "Circle";
	}
	virtual void serialize(ISerializer* serial) const override;
	static void deserialize(IDeserializer* serial, Base* b);

};

static std::map<std::string, std::function<Base* ()>> typesMap = 
{ 
	{"Base", [] {return new Base; }} ,
	{"Point", [] {return new Point; }},
	{"Circle", [] {return new Circle; }}
};
static std::map<std::string, std::function<void (IDeserializer*, Base*)>> deserializeMap =
{
	{"Base", Base::deserialize},
	{"Point", Point::deserialize},
	{"Circle", Circle::deserialize}
};
class OutArchive
{
public:
	OutArchive(std::string fName);
	void write(std::vector<std::shared_ptr<Base>> objects);
private:
	std::unique_ptr<ISerializer> iSerializer;
};
class InArchive
{
public:
	InArchive(std::string fName) 
	{
		iSerializer.reset(new TextDeserializer(fName));
	}
	void read(std::vector<std::shared_ptr<Base>>& objects);
private:
	std::unique_ptr<IDeserializer> iSerializer;
};