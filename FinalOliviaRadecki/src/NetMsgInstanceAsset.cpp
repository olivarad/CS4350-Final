#include <sstream>
#include <string>
#include <memory>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewFinalOliviaRadecki.h"
#include "WorldContainer.h"
#include "NetMsgInstanceAsset.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgInstanceAsset);

NetMsgInstanceAsset::NetMsgInstanceAsset()
{
	this->label;
	this->asset;
	this->defaultRotation;
	this->position;
}

NetMsgInstanceAsset::~NetMsgInstanceAsset()
{

}

bool NetMsgInstanceAsset::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->label;
	os << this->asset.first.first;
	os << this->asset.first.second;
	os << this->asset.second.first;
	os << this->asset.second.second;
	os << this->defaultRotation.first;
	os << this->defaultRotation.second;
	os << this->position.x;
	os << this->position.y;
	os << this->position.z;

	return true;
}

bool NetMsgInstanceAsset::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->label;
	is >> this->asset.first.first;
	is >> this->asset.first.second;
	is >> this->asset.second.first;
	is >> this->asset.second.second;
	is >> this->defaultRotation.first;
	is >> this->defaultRotation.second;
	is >> this->position.x;
	is >> this->position.y;
	is >> this->position.z;

	return true;
}

// Use Data
void NetMsgInstanceAsset::onMessageArrived()
{
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	GLViewFinalOliviaRadecki* glView = ((GLViewFinalOliviaRadecki*)ManagerGLView::getGLViewT<GLViewFinalOliviaRadecki>());
	glView->assets.instanceObject(label, asset, defaultRotation, glView->getWorldContainer(), position);
}

// Print information to terminal
std::string NetMsgInstanceAsset::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << label << " Object Label: " << asset.first.first << " Object Path: " << asset.first.second << " Texture Label: " << asset.second.first << " Texture Path: " << asset.second.second << " X: " << position.x << " Y: " << position.y << " Z: " << position.z << std::endl;

	return ss.str();
}