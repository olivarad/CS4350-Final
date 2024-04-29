#include <sstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewFinalOliviaRadecki.h"
#include "Mat4.h"
#include "NetMsgDeleteAsset.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgDeleteAsset);

NetMsgDeleteAsset::NetMsgDeleteAsset()
{
	this->label;
	this->position;
}

NetMsgDeleteAsset::~NetMsgDeleteAsset()
{

}

bool NetMsgDeleteAsset::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->label;
	os << this->position.x;
	os << this->position.y;
	os << this->position.z;

	return true;
}

bool NetMsgDeleteAsset::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->label;
	is >> this->position.x;
	is >> this->position.y;
	is >> this->position.z;

	return true;
}

// Use Data
void NetMsgDeleteAsset::onMessageArrived()
{
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	GLViewFinalOliviaRadecki* glView = ((GLViewFinalOliviaRadecki*)ManagerGLView::getGLViewT<GLViewFinalOliviaRadecki>());
	glView->assets.deleteAsset(label, position);
}