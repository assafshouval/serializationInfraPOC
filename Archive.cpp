#include "Archive.h"
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <regex>
#include <string>


TextSerializer::TextSerializer(std::string path) : ofstreamObj(path, std::ios::binary)
{
	
}


void TextSerializer::serializeInt(int i)
{
	ofstreamObj << "<int>" << i << "</int>" << std::endl;
}


void TextSerializer::serializeString(std::string s)
{
	//#TODO: escape chars
	ofstreamObj << "<string>" << s << "</string>" << std::endl;
}

void TextSerializer::serializeFloat(float f)
{
	//#TODO: handle floating point precision
	ofstreamObj << "<float>" << f << "</float>" << std::endl;
}

void TextSerializer::serializeObj(std::shared_ptr<Base> b)
{
	ofstreamObj << "<" << b->serializedId() << ">" << std::endl;
	b->serialize(this);
	ofstreamObj << "</" << b->serializedId() << ">" << std::endl;
}

void TextSerializer::serializeObjectsArray(std::vector<std::shared_ptr<Base>> objects)
{
	ofstreamObj << "<ObjectsArray>" << std::endl;
	for (auto pObj : objects)
	{
		serializeObj(pObj);
	}
	ofstreamObj << "</ObjectsArray>" << std::endl;
}

void Base::serialize(ISerializer* serial) const
{
	serial->serializeString(name);
}

void Base::deserialize(IDeserializer* serial, Base* b)
{
	b->name = serial->deserializeString();
}

Point::Point(float _x, float _y) : Base(), x(_x), y(_y) {}

void Point::serialize(ISerializer* serial) const
{
	serial->serializeFloat(x);
	serial->serializeFloat(y);
	__super::serialize(serial);
}

void Point::deserialize(IDeserializer* serial, Base* b)
{
	Point* p = dynamic_cast<Point*>(b);
	p->x = serial->deserializeFloat();
	p->y = serial->deserializeFloat();
	__super::deserialize(serial, b);
}

void Circle::serialize(ISerializer* serial) const
{
	serial->serializeInt(r);
	__super::serialize(serial);
}
//
void Circle::deserialize(IDeserializer* serial, Base* b)
{
	Circle* c = dynamic_cast<Circle*>(b);
	c->r = serial->deserializeInt();
	__super::deserialize(serial, b);
}

OutArchive::OutArchive(std::string fName) 
{
	iSerializer.reset(new TextSerializer(fName));
}

void OutArchive::write(std::vector<std::shared_ptr<Base>> objects)
{
	iSerializer->serializeObjectsArray(objects);
	
}

void InArchive::read(std::vector<std::shared_ptr<Base>>& objects)
{
	iSerializer->deserializeObjectsArray(objects);
}

TextDeserializer::TextDeserializer(std::string fName)
{
	std::ifstream streamObj(fName, std::ios::in);
	std::string xml((std::istreambuf_iterator<char>(streamObj)), std::istreambuf_iterator<char>());
	//clean xml
	std::regex whiteSpaceBetweenChunks(">[\\s\\r\\n]*<");
	stringXML = std::regex_replace(xml, whiteSpaceBetweenChunks, "><");
	it = stringXML.begin();
}

int TextDeserializer::deserializeInt()
{
	std::string s = deserializeToString<int>();
	int i = std::stoi(s);
	return i;
}

std::string TextDeserializer::deserializeString()
{
	//#TODO: de-escape xml characters
	std::string s = deserializeToString<std::string>();
	return s;
}

float TextDeserializer::deserializeFloat()
{
	std::string s = deserializeToString<float>();
	float f = std::stof(s);
	return f;
}

void TextDeserializer::deserializeObjectsArray(std::vector<std::shared_ptr<Base>>& objects)
{
	std::string objectsTag, currObjectTag;
	//#TODO:use generic deserialization  
	std::vector<std::string> objChunks;
	const char* argStart = stringXML.c_str();
	const char* argEnd = stringXML.c_str();
	std::string chunkName = typesTags[std::type_index(typeid(std::vector<std::shared_ptr<Base>>))];
	//#TODO:this is unnecessary anymore (read tag objectsArray, and then read every object on it's own)
	ReadXMLChunk(chunkName, argEnd, stringXML, argStart, objChunks);
	it += size_t(2) + chunkName.length();
	for (auto arg : objChunks)
	{
		std::string strChunkType = arg.substr(1, arg.find_first_of(">") - 1);
		size_t openChunkLen = strChunkType.length() + 2/*<>*/;
		size_t closeChunkLen = strChunkType.length() + 3/*<\>*/;
		it += openChunkLen;
		Base* b = typesMap[strChunkType]();
		std::shared_ptr<Base> bSP;
		bSP.reset(b);
		//since deserialize is static method, we need to map the functions
		deserializeMap[strChunkType](this, b);
		it += closeChunkLen;
		objects.push_back(bSP);
	}
	return;
}

//code adapted after taken from:https://github.com/cpzhang/bud/blob/a37348b77fb2722c73d972a77827056b0b2344f6/trunk/tool/flash/ASInterface.inl#L393
void TextDeserializer::ReadXMLChunk(const std::string& chunkName, const char*& argEnd, const std::string& xml, const char*& argStart, std::vector<std::string>& args)
{
	int nesting = 0;
	bool closetag = false;
	std::string openTag = "<" + chunkName + ">";
	std::string closeTag = "</" + chunkName + ">";
	while (argEnd - xml.c_str() < (int)xml.length())
	{
		if (strncmp(argEnd, closeTag.c_str(), closeTag.length()) == 0) break;
		else
			if (strncmp(argEnd, openTag.c_str(), openTag.length()) == 0)
			{
				nesting = 0;
				argStart += openTag.length();
				argEnd += openTag.length() - 1;
			}
			else
				if (strncmp(argEnd, "</", 2) == 0) closetag = true;
				else
					if (strncmp(argEnd, "<", 1) == 0) ++nesting;
					else
						if (strncmp(argEnd, "/>", 2) == 0) closetag = true;
						else
							if (strncmp(argEnd, ">", 1) == 0)
							{
								if (closetag)
								{
									closetag = false;
									if (--nesting == 0)
									{
										size_t start = argStart - xml.c_str();
										size_t size = argEnd - argStart + 1;
										args.push_back(xml.substr(start, size));
										argStart = argEnd + 1;
									}
								}
							}
		++argEnd;
	}
}

