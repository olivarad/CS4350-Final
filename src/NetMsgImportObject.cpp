#include <sstream>
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

// Use Data
void NetMsgImportObject::onMessageArrived()
{
	std::cout << "Message Arrived" << std::endl;
	std::cout << this->toString();
	GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
	glView->assets.importObjectPath(object);
}

// Print information to terminal
std::string NetMsgImportObject::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << object.first << " Path: " << object.second << std::endl;

	return ss.str();
}