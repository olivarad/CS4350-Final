#include <sstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewNewModule.h"
#include "NetMsgImportTexture.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgImportTexture);

NetMsgImportTexture::NetMsgImportTexture()
{
	this->texture;
}

NetMsgImportTexture::~NetMsgImportTexture()
{

}

bool NetMsgImportTexture::toStream(NetMessengerStreamBuffer& os) const
{
	os << this->texture.first;
	os << this->texture.second;

	return true;
}

bool NetMsgImportTexture::fromStream(NetMessengerStreamBuffer& is)
{
	is >> this->texture.first;
	is >> this->texture.second;

	return true;
}

// Use Data
void NetMsgImportTexture::onMessageArrived()
{
	std::cout << "Message Arrived" << std::endl;
	std::cout << this->toString();
	GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
	glView->assets.importTexturePath(texture);
}

// Print information to terminal
std::string NetMsgImportTexture::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << texture.first << " Path: " << texture.second << std::endl;

	return ss.str();
}