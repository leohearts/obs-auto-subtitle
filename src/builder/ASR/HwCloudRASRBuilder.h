/*
obs-auto-subtitle
 Copyright (C) 2019-2022 Yibai Zhang

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

#ifndef OBS_AUTOSUB_HWCLOUDRASR_BUILDER_H
#define OBS_AUTOSUB_HWCLOUDRASR_BUILDER_H

#include <QString>

#include "../base/ASRBuilderBase.h"
#include "../../vendor/ASR/HwCloudRASR.h"

#define HWCLOUD_IAM_TOKEN_API "https://iam.%1.myhuaweicloud.com/v3/auth/tokens"

class HwCloudRASRBuilder : public ASRBuilderBase {
public:
    void getProperties(obs_properties_t *props);
    void showProperties(obs_properties_t *props);
    void hideProperties(obs_properties_t *props);
    void updateSettings(obs_data_t *settings);
    void getDefaults(obs_data_t *settings);
    ASRBase *build();

protected:

private:
    void refreshToken();

    QString project_id;
    QString region;
    QString username;
    QString password;
    QString domain_name;
    QString token;

    bool needBuild = false;
    bool needRefresh = false;
};


#endif OBS_AUTOSUB_HWCLOUDRASR_BUILDER_H