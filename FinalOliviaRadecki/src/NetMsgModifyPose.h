#pragma once

#include "NetMsg.h"
#include "Mat4.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{
	class NetMsgModifyPose : public NetMsg
	{
	public:
		NetMsgMacroDeclaration(NetMsgModifyPose);

		NetMsgModifyPose();
		virtual ~NetMsgModifyPose();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		// Payload
		std::string label;
		Vector originalPosition;
		Vector position;
		Mat4 pose;

	protected:

	};
}

#endif