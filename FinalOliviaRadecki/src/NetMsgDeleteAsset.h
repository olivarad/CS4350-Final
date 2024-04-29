#pragma once

#include "NetMsg.h"
#include "Mat4.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{
	class NetMsgDeleteAsset : public NetMsg
	{
	public:
		NetMsgMacroDeclaration(NetMsgDeleteAsset);

		NetMsgDeleteAsset();
		virtual ~NetMsgDeleteAsset();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();

		// Payload
		std::string label;
		Vector position;

	protected:

	};
}

#endif