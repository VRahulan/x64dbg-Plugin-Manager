// Copyright (c) 2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "utils.h"

Utils::Utils(QObject *parent) : QObject(parent)
{

}

QList<Utils::RECORD> Utils::getRecords(QString sRootPath)
{
    QList<Utils::RECORD> listResult;

    QFileInfo fi(sRootPath);

    sRootPath=fi.absoluteDir().path()+QDir::separator()+fi.fileName();

    _getRecords(sRootPath,sRootPath,&listResult);

    return listResult;
}

bool Utils::checkMData(Utils::MDATA *pMData, QString *psErrorString)
{
    bool bResult=true;

    if(pMData->sName=="")
    {
        *psErrorString=tr("Invalid name");

        bResult=false;
    }
    // TODO
    return bResult;
}

QString Utils::createBundleName(Utils::MDATA *pMData)
{
    QString sResult;

    sResult+=pMData->sName;

    if(pMData->sCurrentVersion!="")
    {
        sResult+=QString("_%1").arg(pMData->sCurrentVersion);
    }

    if(pMData->sCurrentDate!="")
    {
        sResult+=QString("_%1").arg(pMData->sCurrentDate);
    }

    return sResult;
}

bool Utils::isPluginValid(QString sFileName)
{
    bool bResult=false;

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        bResult=isPluginValid(&file);

        file.close();
    }

    return bResult;
}

bool Utils::isPluginValid(QIODevice *pDevice)
{
    bool bResult=false;

    XZip xzip(pDevice);

    if(xzip.isVaild())
    {
        QList<XArchive::RECORD> listRecords=xzip.getRecords();

        bResult=XArchive::isArchiveRecordPresent("plugin_info.json",&listRecords);
    }

    return bResult;
}

QByteArray Utils::createPluginInfo(Utils::MDATA *pMData, QList<Utils::FILE_RECORD> *pListFileRecords, QList<Utils::DIRECTORY_RECORD> *pListDirectoryRecords, QString sSHA1)
{
    QByteArray baResult;

    QJsonObject recordObject;
    recordObject.insert("Name",             QJsonValue::fromVariant(pMData->sName));
    recordObject.insert("Version",          QJsonValue::fromVariant(pMData->sCurrentVersion));
    recordObject.insert("Date",             QJsonValue::fromVariant(pMData->sCurrentDate));
    recordObject.insert("Author",           QJsonValue::fromVariant(pMData->sAuthor));
    recordObject.insert("Bugreport",        QJsonValue::fromVariant(pMData->sBugreport));
    recordObject.insert("Info",             QJsonValue::fromVariant(pMData->sInfo));
    recordObject.insert("Size",             QJsonValue::fromVariant(pMData->nSize));
    recordObject.insert("CompressedSize",   QJsonValue::fromVariant(pMData->nCompressedSize));

    if(sSHA1=="") // In zip
    {
        int nFilesCount=pListFileRecords->count();
        int nDirectoriesCount=pListDirectoryRecords->count();

        QJsonArray installArray;

        for(int i=0;i<nDirectoriesCount;i++)
        {
            QJsonObject record;

            record.insert("Path",       pListDirectoryRecords->at(i).sPath);
            record.insert("Action",     "make_directory");

            installArray.append(record);
        }

        for(int i=0;i<nFilesCount;i++)
        {
            QJsonObject record;

            record.insert("Path",       pListFileRecords->at(i).sPath);
            record.insert("Action",     "copy_file");
            record.insert("SHA1",       pListFileRecords->at(i).sSHA1);

            installArray.append(record);
        }

        recordObject.insert("Install",  installArray);

        QJsonArray removeArray;

        for(int i=0;i<nFilesCount;i++)
        {
            QJsonObject record;

            record.insert("Path",       pListFileRecords->at(i).sPath);
            record.insert("Action",     "remove_file");

            removeArray.append(record);
        }

        for(int i=0;i<nDirectoriesCount;i++)
        {
            QJsonObject record;

            record.insert("Path",       pListDirectoryRecords->at(i).sPath);
            record.insert("Action",     "remove_directory_if_empty");

            removeArray.append(record);
        }

        recordObject.insert("Remove",   removeArray);
    }
    else
    {
        recordObject.insert("Src",          "");
        recordObject.insert("SHA1",         QJsonValue::fromVariant(sSHA1));
    }

    QJsonDocument doc(recordObject);
    baResult.append(doc.toJson());

    return baResult;
}

Utils::MDATA Utils::getMDataFromZip(QString sFileName, QString sRootPath)
{
    Utils::MDATA result={};

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        result=getMDataFromZip(&file,sRootPath);

        file.close();
    }

    return result;
}

Utils::MDATA Utils::getMDataFromZip(QIODevice *pDevice, QString sRootPath)
{
    Utils::MDATA result={};

    result.sRoot=sRootPath;

    XZip xzip(pDevice);

    if(xzip.isVaild())
    {
        QList<XArchive::RECORD> listRecords=xzip.getRecords();

        XArchive::RECORD pluginInfoRecord=XArchive::getArchiveRecord("plugin_info.json",&listRecords);

        QByteArray baData=xzip.decompress(&pluginInfoRecord);

        result=getMDataFromData(baData,sRootPath);
    }

    return result;
}

Utils::MDATA Utils::getMDataFromData(QByteArray baData, QString sRootPath)
{
    Utils::MDATA result={};

    QJsonDocument jsDoc=QJsonDocument::fromJson(baData);

    QJsonObject rootObj=jsDoc.object();

    result.sName            =rootObj.value("Name").toString();
    result.sCurrentVersion         =rootObj.value("Version").toString();
    result.sCurrentDate            =rootObj.value("Date").toString();
    result.sAuthor          =rootObj.value("Author").toString();
    result.sBugreport       =rootObj.value("Bugreport").toString();
    result.sInfo            =rootObj.value("Info").toString();
    result.nSize            =rootObj.value("Size").toInt();
    result.nCompressedSize  =rootObj.value("CompressedSize").toInt();

    QJsonArray installArray=rootObj.value("Install").toArray();

    int nCount=installArray.count();

    for(int i=0;i<nCount;i++)
    {
        QJsonObject recordObj=installArray.at(i).toObject();
        RECORD record={};

        record.sPath        =recordObj.value("Path").toString();
        record.sFullPath    =sRootPath+QDir::separator()+record.sPath;
        record.sSHA1        =recordObj.value("SHA1").toString();

        QString sAction=recordObj.value("Action").toString();

        if(sAction=="copy_file")
        {
            record.bIsFile=true;
        }

        result.listRecords.append(record);
    }

    return result;
}

QList<Utils::MDATA> Utils::getInstalledModules(QString sDataPath, QString sRootPath)
{
    QList<Utils::MDATA> listResult;

    QDir dir(sDataPath+QDir::separator()+"installed");

    QFileInfoList listFiles=dir.entryInfoList(QStringList()<<"*.json",QDir::Files|QDir::NoSymLinks,QDir::Name);

    int nCount=listFiles.count();

    for(int i=0;i<nCount;i++)
    {
        QString sBundleName=listFiles.at(i).filePath();

        QFile file;
        file.setFileName(sBundleName);

        if(file.open(QIODevice::ReadOnly))
        {
            QByteArray baData=file.readAll();

            Utils::MDATA record=getMDataFromData(baData,sRootPath);
            record.sBundlePath=sBundleName;

            listResult.append(record);

            file.close();
        }
    }

    return listResult;
}

QList<Utils::MDATA> Utils::getModulesFromJSONFile(QString sJSONFilePath)
{
    QList<Utils::MDATA> listResult;
    // TODO

    return listResult;
}

QList<Utils::MDATA> Utils::mergeMData(QList<Utils::MDATA> *pList1, QList<Utils::MDATA> *pList2)
{
    QList<Utils::MDATA> listResult;

    listResult.append(*pList1);

    // TODO

    return listResult;
}

void Utils::_getRecords(QString sRootPath, QString sCurrentPath, QList<Utils::RECORD> *pListRecords)
{
    QFileInfo fi(sCurrentPath);

    RECORD record={};

    record.bIsFile=true;
    record.sFullPath=fi.absoluteFilePath();
    record.sPath=record.sFullPath;
    record.bIsFile=fi.isFile();

    record.sPath=record.sPath.remove(0,sRootPath.size()+1);

    if(record.sPath!="")
    {
        pListRecords->append(record);
    }

    if(!record.bIsFile)
    {
        QDir dir(sCurrentPath);
        QFileInfoList eil=dir.entryInfoList();

        int nCount=eil.count();

        for(int i=0;i<nCount;i++)
        {
            QString sFN=eil.at(i).fileName();

            if((sFN!=".")&&(sFN!=".."))
            {
                _getRecords(sRootPath,eil.at(i).absoluteFilePath(),pListRecords);
            }
        }
    }
}
