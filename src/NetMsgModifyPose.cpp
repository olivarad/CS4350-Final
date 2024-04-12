#include <sstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewNewModule.h"
#include "Mat4.h"
#include "NetMsgModifyPose.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgModifyPose);

NetMsgModifyPose::NetMsgModifyPose()
{
	this->label;
	this->position;
	this->pose;
}

NetMsgModifyPose::~NetMsgModifyPose()
{

}

bool NetMsgModifyPose::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->label;
	os << this->position.x;
	os << this->position.y;
	os << this->position.z;
	for (int i = 0; i < 15; ++i)
	{
		os << this->pose.at(i);
	}

	return true;
}

bool NetMsgModifyPose::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->label;
	is >> this->position.x;
	is >> this->position.y;
	is >> this->position.z;
	for (int i = 0; i < 15; ++i)
	{
		is >> this->pose.at(i);
	}

	return true;
}

// Use Data
void NetMsgModifyPose::onMessageArrived()
{
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
	glView->assets.modifyPose(label, position, pose);
}

// Print information to terminal
std::string NetMsgModifyPose::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << label << " X: " << position.x << " Y: " << position.y << " Z: " << position.z << " Matrix: " << pose.toString() << std::endl;

	return ss.str();
}