/*
obs-auto-subtitle
 Copyright (C) 2019-2020 Yibai Zhang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; If not, see <https://www.gnu.org/licenses/>
*/

#include <time.h>
#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <utility>

#include "HwCloudRASR.h"

using namespace std::placeholders;

HwCloudRASR::HwCloudRASR(const QString &region, const QString &project_id,
			 const QString &token, QObject *parent)
	: ASRBase(parent), region(region), project_id(project_id), token(token)
{
	connect(&ws, &QWebSocket::connected, this, &HwCloudRASR::onConnected);
	connect(&ws, &QWebSocket::disconnected, this,
		&HwCloudRASR::onDisconnected);
	connect(&ws, &QWebSocket::textMessageReceived, this,
		&HwCloudRASR::onTextMessageReceived);
	connect(this, &HwCloudRASR::haveResult, this, &HwCloudRASR::onResult);
	connect(&ws, SIGNAL(error(QAbstractSocket::SocketError)), this,
		SLOT(onError(QAbstractSocket::SocketError)));

	running = false;
}
static const char *startMsg = "{\n"
			      "  \"command\": \"START\",\n"
			      "  \"config\":\n"
			      "  {\n"
			      "    \"audio_format\": \"pcm16k16bit\",\n"
			      "    \"property\": \"chinese_16k_general\",\n"
			      "    \"add_punc\": \"yes\",\n"
			      "    \"vad_tail\": 300,\n"
			      "    \"max_second\": 15, \n"
			      "    \"interim_results\": \"yes\"\n"
			      "  }\n"
			      "}";

static const char *endMsg = "{\n"
			    "  \"command\": \"END\",\n"
			    "  \"cancel\": false\n"
			    "}";

void HwCloudRASR::onStart()
{
	auto uri = QString(HWCLOUD_SIS_RASR_URI).arg(project_id);
	QNetworkRequest request;
	auto urlStr = QString("wss://") +
		      QString(HWCLOUD_SIS_ENDPOINT).arg(region) + uri;
	QUrl url(urlStr);
	qDebug() << url.toString();
	request.setUrl(url);
	request.setRawHeader("X-Auth-Token", token.toLocal8Bit());
	ws.open(request);
}

void HwCloudRASR::onError(QAbstractSocket::SocketError error)
{
	auto errorCb = getErrorCallback();
	if (errorCb)
		errorCb(ERROR_SOCKET, ws.errorString());
	qDebug() << ws.errorString();
}

void HwCloudRASR::onConnected()
{
	ws.sendTextMessage(startMsg);
	auto connectCb = getConnectedCallback();
	if (connectCb)
		connectCb();
	qDebug() << "WebSocket connected";
	running = true;
	connect(this, &ASRBase::sendAudioMessage, this,
		&HwCloudRASR::onSendAudioMessage);
}

void HwCloudRASR::onDisconnected()
{
	running = false;
	auto disconnectCb = getDisconnectedCallback();
	if (disconnectCb)
		disconnectCb();
	qDebug() << "WebSocket disconnected";
}

void HwCloudRASR::onSendAudioMessage(const char *data, unsigned long size)
{
	if (!running) {
		return;
	}
	ws.sendBinaryMessage(QByteArray::fromRawData((const char *)data, size));
}

void HwCloudRASR::onTextMessageReceived(const QString message)
{
	QJsonDocument doc(QJsonDocument::fromJson(message.toUtf8()));
	if (doc["resp_type"].toString() != "RESULT") {
		if (doc["resp_type"].toString() == "ERROR") {
			auto errorCb = getErrorCallback();
			if (errorCb)
				errorCb(ERROR_API, doc["error_msg"].toString());
		}
		qDebug() << message;
		return;
	}
	auto segments = doc["segments"];
	if (!segments.isArray()) {
		return;
	}
	auto seg = segments[0];
	auto is_final = seg["is_final"].toBool(true);
	auto output = seg["result"]["text"].toString();
	emit haveResult(output, is_final ? ResultType_End : ResultType_Middle);
}

void HwCloudRASR::onResult(QString message, int type)
{
	auto callback = getResultCallback();
	if (callback)
		callback(message, type);
}

void HwCloudRASR::onStop()
{
	ws.sendTextMessage(QString(endMsg).toUtf8());
	ws.close();
}

QString HwCloudRASR::getProjectId()
{
	return project_id;
}

QString HwCloudRASR::getToken()
{
	return token;
}

HwCloudRASR::~HwCloudRASR()
{
	stop();
}
