#pragma once

#include "NetMsg.h"
#include "WorldContainer.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{

	typedef std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> ObjectandTexture;

	class NetMsgInstanceAsset : public NetMsg
	{
	public:
		NetMsgMacroDeclaration(NetMsgInstanceAsset);

		NetMsgInstanceAsset();
		virtual ~NetMsgInstanceAsset();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		// Payload
		std::string label;
		ObjectandTexture asset;
		std::pair<int, int> defaultRotation;
		Vector position;

	protected:

	};
}

#endif