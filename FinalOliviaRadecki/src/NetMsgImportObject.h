#pragma once

#include "NetMsg.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{
	class NetMsgImportObject : public NetMsg
	{
	public:
		NetMsgMacroDeclaration(NetMsgImportObject);

		NetMsgImportObject();
		virtual ~NetMsgImportObject();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		// Payload
		std::pair<std::string, std::string> object; // Label and Path, assumes texture files have the same absolute path

	protected:

	};
}

#endif