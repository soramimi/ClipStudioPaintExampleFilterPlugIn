//! Hello World Filter Plugin for CLIP STUDIO EX
//! Copyright (C) @soramimi_jp

#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include <vector>

static const int kStringIDFilterCategoryName		= 101;
static const int kStringIDFilterName				= 102;

struct HelloWorldFilterInfo {
	TriglavPlugInPropertyService *pPropertyService;
};

// plugin main
void TRIGLAV_PLUGIN_API TriglavPluginCall(TriglavPlugInInt *result, TriglavPlugInPtr *data, TriglavPlugInInt selector, TriglavPlugInServer *pluginServer, TriglavPlugInPtr reserved)
{
	*result	= kTriglavPlugInCallResultFailed;
	if (pluginServer) {
		if (selector == kTriglavPlugInSelectorModuleInitialize) {
			// initialize plugin
			TriglavPlugInModuleInitializeRecord *pModuleInitializeRecord = pluginServer->recordSuite.moduleInitializeRecord;
			TriglavPlugInStringService *pStringService = pluginServer->serviceSuite.stringService;
			if (pModuleInitializeRecord && pStringService) {
				TriglavPlugInInt hostVersion;
				pModuleInitializeRecord->getHostVersionProc(&hostVersion, pluginServer->hostObject);
				if (hostVersion >= kTriglavPlugInNeedHostVersion) {
					TriglavPlugInStringObject moduleID = nullptr;
					const char *moduleIDString = "78B78B66-F64A-4B5D-9804-8A11B7ED48AE";
					pStringService->createWithAsciiStringProc(&moduleID, moduleIDString, static_cast<TriglavPlugInInt>(::strlen(moduleIDString)));
					(*pModuleInitializeRecord).setModuleIDProc(pluginServer->hostObject, moduleID);
					(*pModuleInitializeRecord).setModuleKindProc(pluginServer->hostObject, kTriglavPlugInModuleSwitchKindFilter);
					pStringService->releaseProc(moduleID);

					HelloWorldFilterInfo *pFilterInfo = new HelloWorldFilterInfo;
					*data = pFilterInfo;
					*result	= kTriglavPlugInCallResultSuccess;
				}
			}
		} else if (selector == kTriglavPlugInSelectorModuleTerminate) {
			// terminate plugin
			HelloWorldFilterInfo *pFilterInfo = static_cast<HelloWorldFilterInfo *>(*data);
			delete pFilterInfo;
			*data = nullptr;
			*result = kTriglavPlugInCallResultSuccess;
		} else if(selector == kTriglavPlugInSelectorFilterInitialize) {
			// initialize filter
			TriglavPlugInRecordSuite *pRecordSuite = &pluginServer->recordSuite;
			TriglavPlugInHostObject hostObject = pluginServer->hostObject;
			TriglavPlugInStringService *pStringService = pluginServer->serviceSuite.stringService;
			TriglavPlugInPropertyService *pPropertyService = pluginServer->serviceSuite.propertyService;
			if (TriglavPlugInGetFilterInitializeRecord(pRecordSuite) && pStringService && pPropertyService) {
				// set filter category and name
				TriglavPlugInStringObject filterCategoryName = nullptr;
				TriglavPlugInStringObject filterName = nullptr;
				pStringService->createWithStringIDProc(&filterCategoryName, kStringIDFilterCategoryName, pluginServer->hostObject);
				pStringService->createWithStringIDProc(&filterName, kStringIDFilterName, pluginServer->hostObject);
				
				// access keys
				TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite, hostObject, filterCategoryName, 'c');
				TriglavPlugInFilterInitializeSetFilterName(pRecordSuite, hostObject, filterName, 'h');

				pStringService->releaseProc(filterCategoryName);
				pStringService->releaseProc(filterName);

				// target
				TriglavPlugInInt target[]={
					kTriglavPlugInFilterTargetKindRasterLayerGrayAlpha,
					kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha,
					kTriglavPlugInFilterTargetKindRasterLayerAlpha
				};
				TriglavPlugInFilterInitializeSetTargetKinds(pRecordSuite, hostObject, target, 3);

				*result	= kTriglavPlugInCallResultSuccess;
			}
		} else if (selector == kTriglavPlugInSelectorFilterTerminate) {
			// terminate filter
			*result	= kTriglavPlugInCallResultSuccess;
		} else if (selector == kTriglavPlugInSelectorFilterRun) {
			// run filter
			TriglavPlugInRecordSuite *pRecordSuite = &pluginServer->recordSuite;
			TriglavPlugInOffscreenService *pOffscreenService = pluginServer->serviceSuite.offscreenService;
			TriglavPlugInPropertyService *pPropertyService = pluginServer->serviceSuite.propertyService;
			TriglavPlugInBitmapService *pBitmapService = pluginServer->serviceSuite.bitmapService;

			if (TriglavPlugInGetFilterRunRecord(pRecordSuite) && pOffscreenService && pPropertyService && pBitmapService ) {
				TriglavPlugInPropertyObject propertyObject;
				TriglavPlugInFilterRunGetProperty(pRecordSuite, &propertyObject, pluginServer->hostObject);

				TriglavPlugInOffscreenObject sourceOffscreenObject;
				TriglavPlugInFilterRunGetSourceOffscreen(pRecordSuite, &sourceOffscreenObject, pluginServer->hostObject);

				TriglavPlugInOffscreenObject destinationOffscreenObject;
				TriglavPlugInFilterRunGetDestinationOffscreen(pRecordSuite, &destinationOffscreenObject, pluginServer->hostObject);

				TriglavPlugInRect selectAreaRect;
				TriglavPlugInFilterRunGetSelectAreaRect(pRecordSuite, &selectAreaRect, pluginServer->hostObject);

				TriglavPlugInOffscreenObject selectAreaOffscreenObject;
				TriglavPlugInFilterRunGetSelectAreaOffscreen(pRecordSuite, &selectAreaOffscreenObject, pluginServer->hostObject);

				TriglavPlugInInt chR, chG, chB;
				pOffscreenService->getRGBChannelIndexProc(&chR, &chG, &chB, destinationOffscreenObject);
				
				TriglavPlugInInt	blockRectCount;
				pOffscreenService->getBlockRectCountProc(&blockRectCount, destinationOffscreenObject, &selectAreaRect);
				
				std::vector<TriglavPlugInRect>	blockRects;
				blockRects.resize(blockRectCount);
				for (TriglavPlugInInt i = 0; i < blockRectCount; i++) {
					pOffscreenService->getBlockRectProc(&blockRects[i], i, destinationOffscreenObject, &selectAreaRect);
				}

				TriglavPlugInFilterRunSetProgressTotal(pRecordSuite, pluginServer->hostObject, blockRectCount);

				HelloWorldFilterInfo *pFilterInfo = static_cast<HelloWorldFilterInfo *>(*data);
				pFilterInfo->pPropertyService = pPropertyService;

				bool restart = true;

				while (1) {

					TriglavPlugInInt processResult;
					if (restart) {
						restart	= false;
						TriglavPlugInFilterRunProcess(pRecordSuite, &processResult, pluginServer->hostObject, kTriglavPlugInFilterRunProcessStateStart);
						if (processResult == kTriglavPlugInFilterRunProcessResultExit) {
							break;
						}

						int depth = 0;
						TriglavPlugInInt chorder;
						pOffscreenService->getChannelOrderProc(&chorder, sourceOffscreenObject);
						switch (chorder) {
						case kTriglavPlugInOffscreenChannelOrderAlpha:
							depth = 1;
							break;
						case kTriglavPlugInOffscreenChannelOrderGrayAlpha:
							depth = 2;
							break;
						case kTriglavPlugInOffscreenChannelOrderRGBAlpha:
							depth = 4;
							break;
						}
						if (depth != 0) {
							TriglavPlugInOffscreenObject ossel;
							pRecordSuite->filterRunRecord->getSelectAreaOffscreenProc(&ossel, pluginServer->hostObject);

							TriglavPlugInBitmapObject bmsel = nullptr;
							TriglavPlugInBitmapObject bmobj = nullptr;
							int w = selectAreaRect.right - selectAreaRect.left;
							int h = selectAreaRect.bottom - selectAreaRect.top;
							pBitmapService->createProc(&bmobj, w, h, depth, kTriglavPlugInBitmapScanlineHorizontalLeftTop);
							TriglavPlugInPoint bmpos;
							TriglavPlugInPoint ospos;
							bmpos.x = 0;
							bmpos.y = 0;
							ospos.x = selectAreaRect.left;
							ospos.y = selectAreaRect.top;
							pOffscreenService->getBitmapProc(bmobj, &bmpos, sourceOffscreenObject, &ospos, w, h, kTriglavPlugInOffscreenCopyModeNormal);
							if (selectAreaOffscreenObject) {
								pBitmapService->createProc(&bmsel, w, h, 1, kTriglavPlugInBitmapScanlineHorizontalLeftTop);
								pOffscreenService->getBitmapProc(bmsel, &bmpos, ossel, &ospos, w, h, kTriglavPlugInOffscreenCopyModeNormal);
							}

							TriglavPlugInInt bytespixel;
							pBitmapService->getPixelBytesProc(&bytespixel, bmobj);

							for (int y = 0; y < h; y++) {
								TriglavPlugInPoint pos;
								pos.x = 0;
								pos.y = y;
								uint8_t *p, *s;
								pBitmapService->getAddressProc((TriglavPlugInPtr *)&p, bmobj, &pos);
								if (bmsel) {
									pBitmapService->getAddressProc((TriglavPlugInPtr *)&s, bmsel, &pos);
								} else {
									s = nullptr;
								}
								switch (chorder) {
								case kTriglavPlugInOffscreenChannelOrderAlpha:
									for (int x = 0; x < w; x++) {
										uint8_t sY = 128;
										uint8_t S = 255;
										if (s) {
											S = *s++;
										}
										uint8_t dY = p[0];
										if (S == 255) {
											dY = sY;
										} else {
											int R = sY * S + dY * (255 - S);
											dY = R / 255;
										}
										p[0] = dY;
										p += bytespixel;
									}
									break;
								case kTriglavPlugInOffscreenChannelOrderGrayAlpha:
									for (int x = 0; x < w; x++) {
										uint8_t sY = 0xff;
										uint8_t sA = 0xff;
										uint8_t S = 255;
										if (s) {
											S = *s++;
										}
										if (sA > 0 || S == 0) {
											uint8_t dY = p[0];
											uint8_t dA = p[1];
											S = S * sA / 255;
											if (dA == 0 || S == 255) {
												dY = sY;
												dA = S;
											} else {
												int R = sY * S * 255 + dY * dA * (255 - S);
												int A = S * 255 + dA * (255 - S);
												dY = R / A;
												dA = A / 255;
											}
											p[0] = dY;
											p[1] = dA;
										}
										p += bytespixel;
									}
									break;
								case kTriglavPlugInOffscreenChannelOrderRGBAlpha:
									for (int x = 0; x < w; x++) {
										uint8_t sR = 0xff;
										uint8_t sG = 0x00;
										uint8_t sB = 0xff;
										uint8_t sA = 0xff;
										uint8_t S = 255;
										if (s) {
											S = *s++;
										}
										if (sA > 0 || S == 0) {
											uint8_t dR = p[chR];
											uint8_t dG = p[chG];
											uint8_t dB = p[chB];
											uint8_t dA = p[3];
											S = S * sA / 255;
											if (dA == 0 || S == 255) {
												dR = sR;
												dG = sG;
												dB = sB;
												dA = S;
											} else {
												int R = sR * S * 255 + dR * dA * (255 - S);
												int G = sG * S * 255 + dG * dA * (255 - S);
												int B = sB * S * 255 + dB * dA * (255 - S);
												int A = S * 255 + dA * (255 - S);
												dR = R / A;
												dG = G / A;
												dB = B / A;
												dA = A / 255;
											}
											p[chR] = dR;
											p[chG] = dG;
											p[chB] = dB;
											p[3] = dA;
										}
										p += bytespixel;
									}
									break;
								}
							}

							pOffscreenService->setBitmapProc(destinationOffscreenObject, &ospos, bmobj, &bmpos, w, h, kTriglavPlugInOffscreenCopyModeNormal);
							pBitmapService->releaseProc(bmobj);

							TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite, pluginServer->hostObject, &selectAreaRect);
						}
						TriglavPlugInFilterRunProcess(pRecordSuite, &processResult, pluginServer->hostObject, kTriglavPlugInFilterRunProcessStateEnd);
					}

					if (processResult == kTriglavPlugInFilterRunProcessResultRestart) {
						restart	= true;
					} else if (processResult == kTriglavPlugInFilterRunProcessResultExit) {
						break;
					}
				}
				*result	= kTriglavPlugInCallResultSuccess;
			}
		}
	}
	return;
}

