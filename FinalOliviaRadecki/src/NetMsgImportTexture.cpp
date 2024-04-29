#include <sstream>
#include <fstream>
#include <string>
#include "ManagerSerializableNetMsgMap.h"
#include "NetMessengerStreamBuffer.h"
#include "NetMsgMacroMethods.h"
#include "NetMsg_callback_decl.h"
#include "NetMsg.h"
#include "ManagerGLView.h"
#include "GLViewFinalOliviaRadecki.h"
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
	//std::cout << "Message Arrived" << std::endl;
	//std::cout << this->toString();
	if (texture.second.substr(texture.second.length() - 3) != "jpg")
		return;
	std::ifstream textureFile;
	textureFile.open(texture.second);
	if (textureFile.good())
	{
		GLViewFinalOliviaRadecki* glView = ((GLViewFinalOliviaRadecki*)ManagerGLView::getGLViewT<GLViewFinalOliviaRadecki>());
		glView->assets.importTexturePath(texture);
	}
	else 
	{
		// Send request for texture and add it to this computer's multimedia directory
	}
	textureFile.close();
}

// Print information to terminal
std::string NetMsgImportTexture::toString() const
{
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "Label: " << texture.first << " Path: " << texture.second << std::endl;

	return ss.str();
}