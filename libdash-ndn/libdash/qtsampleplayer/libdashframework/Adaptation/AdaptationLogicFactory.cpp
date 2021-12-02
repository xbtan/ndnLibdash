/*
 * AdaptationLogicFactory.cpp
 *****************************************************************************
 * Copyright (C) 2013, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "AdaptationLogicFactory.h"

using namespace libdash::framework::adaptation;
using namespace dash::mpd;

IAdaptationLogic* AdaptationLogicFactory::Create(LogicType logic, IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet)
{
    switch(logic)
    {
        case Manual:            return new ManualAdaptation     (mpd, period, adaptationSet);
        case AlwaysLowest:      return new AlwaysLowestLogic    (mpd, period, adaptationSet);
	case RateBase:		return new RateBaseAdaptation   (mpd, period, adaptationSet);
        case RateBufferBase:    return new RateBufferBaseAdaptation   (mpd, period, adaptationSet);
        case QoeFair:		return new QoeFairAdaptation   (mpd, period, adaptationSet);
        case Game:              return new GameAdaptation      (mpd, period, adaptationSet);
        default:                return new ManualAdaptation     (mpd, period, adaptationSet);
    }
}
