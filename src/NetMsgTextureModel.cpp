#include <sstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewNewModule.h"
#include "NetMsgTextureModel.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgTextureModel);

NetMsgTextureModel::NetMsgTextureModel()
{
	this->object;
	this->texture;
	this->category;
	this->defaultRotation;
}

NetMsgTextureModel::~NetMsgTextureModel()
{

}

bool NetMsgTextureModel::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->object.first;
	os << this->object.second;
	os << this->texture.first;
	os << this->texture.second;
	os << this->category;
	os << this->defaultRotation.first;
	os << this->defaultRotation.second;

	return true;
}

bool NetMsgTextureModel::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->object.first;
	is >> this->object.second;
	is >> this->texture.first;
	is >> this->texture.second;
	is >> this->category;
	is >> this->defaultRotation.first;
	is >> this->defaultRotation.second;

	return true;
}

// Use Data
void NetMsgTextureModel::onMessageArrived()
{
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
	glView->assets.textureModel(object, texture, category, defaultRotation);
}

// Print information to terminal
std::string NetMsgTextureModel::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Object Label: " << object.first << " Object Path: " << object.second << " Texture Label: " << texture.first << " Texture Path: " << texture.second << std::endl;

	return ss.str();
}