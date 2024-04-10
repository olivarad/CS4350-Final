#pragma once

#include "NetMsg.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{
	class NetMsgTextureModel : public NetMsg
	{
	public:
		NetMsgMacroDeclaration(NetMsgTextureModel);

		NetMsgTextureModel();
		virtual ~NetMsgTextureModel();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		// Payload
		std::pair<std::string, std::string> object; // Label and Path, assumes texture files have the same absolute path
		std::pair<std::string, std::string> texture; // Label and Path, assumes texture files have the same absolute path
		std::pair<int, int> defaultRotation;

	protected:

	};
}

#endif