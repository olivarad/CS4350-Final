#include <sstream>
#include <fstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewNewModule.h"
#include "NetMsgImportObject.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgImportObject);

NetMsgImportObject::NetMsgImportObject()
{ 
	this->object;
}

NetMsgImportObject::~NetMsgImportObject()
{

}

bool NetMsgImportObject::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->object.first;
	os << this->object.second;

	return true;
}

bool NetMsgImportObject::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->object.first;
	is >> this->object.second;

	return true;
}

void NetMsgImportObject::onMessageArrived()
{
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	if (object.second.substr(object.second.length() - 3) != "obj")
		return;
	std::ifstream objectFile;
	objectFile.open(object.second);
	if (objectFile.good())
	{
		GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
		glView->assets.importObjectPath(object);
	}
	else
	{
		// Send request for object and add it to this computer's multimedia directory
	}
	objectFile.close();
}

// Print information to terminal
std::string NetMsgImportObject::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << object.first << " Path: " << object.second << std::endl;

	return ss.str();
}