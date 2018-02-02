//
//  CompressRes.cpp
//  mahjonghubei
//
//  Created by jlb on 2017/12/4.
//

#if !defined(_WIN32)

#include "CompressRes.hpp"

#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

CompressRes::CompressRes()
{
}

void CompressRes::setDecodeFlag(int decodeFlag)
{
    this->m_nDecodeFlag = decodeFlag;
}

void CompressRes::setCompressFileName(const char *fileName)
{
    this->m_compressFileName = fileName;
}

std::string CompressRes::getCompressFileName()
{
    return this->m_compressFileName;
}

void CompressRes::addCompressDir(string dirName)
{
    m_dirList.push_back(dirName);
}

bool CompressRes::Compress()
{
    string _rootPath = getcwd(NULL, NULL);
    string resCompressPath = _rootPath + "/" + this->m_compressFileName;
    
    //清空文件
    FILE *compressFile = fopen(resCompressPath.c_str(),"w");
    fclose(compressFile);
    
    for(std::vector<string>::iterator it = m_dirList.begin(); it != m_dirList.end(); it++)
    {
        string dirPath = _rootPath + "/" + (*it);
        
        FILE *addSrcOpenFile = fopen(resCompressPath.c_str(),"a+");
        
        bool isDir = true;
        struct dirent *dirp;
        DIR* dir = opendir(dirPath.c_str());
        if(dir == NULL)
        {
            isDir = false;
        }
        else
        {
            closedir(dir);
        }
        
        if(isDir)
        {
            this->compressDir(dirPath.c_str(), addSrcOpenFile, (*it).c_str());
        }
        else
        {
            this->compressFile(dirPath.c_str(), addSrcOpenFile, (*it).c_str());
        }
        
        fclose(addSrcOpenFile);
    }
    
    return true;
}

bool CompressRes::DeCompress(const char *compressPath, const char *destWritePath)
{
    unsigned int fileContentLen = 0;
    char *buff = this->readBinaryFile((char *)compressPath, fileContentLen);
    
    this->DeCompress((unsigned char *)buff, (long)fileContentLen, destWritePath);
    
    return true;
}

bool  CompressRes::DeCompress(unsigned char *fileContent, long fileSize, const char *destWritePath)
{
    unsigned char *curPos = fileContent;
    while (true)
    {
        int pathLen = 0;
        memcpy(&pathLen, curPos, sizeof(unsigned int));
        curPos = curPos + sizeof(unsigned int);
        
        char *filePath = (char *)malloc(pathLen);
        snprintf(filePath, pathLen, "%s", curPos);
        curPos = curPos + pathLen;
        COMPRESS_LOG("CompressRes::DeCompress = %s\n", filePath);
        
        int fileConentLen = 0;
        memcpy(&fileConentLen, curPos, sizeof(unsigned int));
        curPos = curPos + sizeof(unsigned int);
        
        char *fileContentBuf = (char*)malloc(fileConentLen);
        memcpy(fileContentBuf, curPos, fileConentLen);
        curPos = curPos + fileConentLen;
        
        char *encodeContent = (char *)malloc(fileConentLen);
        this->encode((const unsigned char*)fileContentBuf, fileConentLen, encodeContent);
        
        char fileWritePath[1024] = { 0 };
        sprintf(fileWritePath, "%s%s", destWritePath, filePath);
        FILE *newFile;
        createFile(fileWritePath, &newFile);
        fwrite(encodeContent, fileConentLen, 1, newFile);
        fclose(newFile);
        
        if (curPos - fileContent >= fileSize)
        {
            break;
        }
    }
    
    return true;
}

void CompressRes::compressDir(const char *sourceDirPath, FILE *destCompressFile, const char *sourceDirName)
{
    struct dirent *dirp;
    DIR* dir = opendir(sourceDirPath);
    
    while ((dirp = readdir(dir)) != nullptr) {
        if (dirp->d_type == DT_REG) {
            if (strcmp(dirp->d_name, ".DS_Store") == 0)
            {
                continue;
            }
            
            // 全路径
            char tempPath[1024] = {0};
            sprintf(tempPath, "%s/%s", sourceDirPath, dirp->d_name);
            
            // 获取sourceDirName开头的相对路径
            char dirNameBuf[1024] = {0};
            sprintf(dirNameBuf, "/%s/", sourceDirName);
            char filePath[1024] = {0};
            getRelativeDirPath(tempPath, dirNameBuf, filePath);
            
            if(this->writeData(tempPath, destCompressFile, filePath) == false){
                break;
            }
        } else if (dirp->d_type == DT_DIR) {
            // 文件夹
            if(strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
            {
                char tempPath[1024] = {0};
                sprintf(tempPath, "%s/%s", sourceDirPath, dirp->d_name);
                this->compressDir(tempPath, destCompressFile, sourceDirName);
            }
        }
    }
    
    closedir(dir);
}

void CompressRes::compressFile(const char *filePath, FILE *destCompressFile, const char *fileName)
{
    this->writeData((char*)filePath, destCompressFile, fileName);
}

bool CompressRes::writeData(char *filePath, FILE *destCompressFile, const char *fileName)
{
    unsigned int pathLen = strlen(fileName) + 1;  // +1表示写入末尾的\0
    if (fwrite(&pathLen, sizeof(unsigned int), 1, destCompressFile) != 1) // 写入文件路径长度
    {
        COMPRESS_LOG("pathLen write error");
        return false;
    }
    
    if (fwrite(fileName, 1, pathLen, destCompressFile) != pathLen)  // 写入文件路径
    {
        COMPRESS_LOG("tempPath write error");
        return false;
    }
    COMPRESS_LOG("CompressRes  %s\n", fileName);
    
    unsigned int fileContentLen = 0;
    char *contentBuf = readBinaryFile(filePath, fileContentLen);
    char *encodeContent = (char *)malloc(fileContentLen);
    this->encode((const unsigned char*)contentBuf, fileContentLen, encodeContent);
    // 写入文件数据大小
    if (fwrite(&fileContentLen, sizeof(unsigned int), 1, destCompressFile) != 1)
    {
        COMPRESS_LOG("fileContentLen write error");
        return false;
    }
    
    // 写入文件数据
    if (fwrite(encodeContent, fileContentLen, 1, destCompressFile) != 1)
    {
        COMPRESS_LOG("contentBuf write error");
        return false;
    }
    
    free(contentBuf);
    free(encodeContent);
    
    return true;
}

char *CompressRes::readBinaryFile(char* fname, unsigned int &size)
{
    FILE* imgP;
    imgP = fopen(fname,"rb");//这里是用二进制读取，read-r；binary-b；
    if (imgP == NULL)
        return NULL;
    fseek(imgP, 0L, SEEK_END);
    unsigned int filesize = ftell(imgP);
    size = filesize;
    char *imgbuf = new char[filesize+ 1];
    fseek(imgP,0x0L,SEEK_SET);//图片源
    fread(imgbuf, sizeof(imgbuf[0]), size, imgP);
    fclose(imgP);
    
    return imgbuf;
}

bool CompressRes::createFile(char *filepath, FILE **newFILE)
{
    char *startPos = strchr(filepath, '/');
    char *lastPos = startPos;
    while(true)
    {
        char *curPos = strchr(lastPos+1, '/');
        if (curPos == NULL)
        {
            *newFILE = fopen(filepath, "w");
            break;
        }
        else // 是目录
        {
            char dirPath[1024] = {0};
            strncpy(dirPath, startPos, (curPos-startPos));
            struct stat fileStat;
            
            bool isExist = false;
            if ((stat(dirPath, &fileStat) == 0) && S_ISDIR(fileStat.st_mode)) // 目录已存在
            {
            }
            else
            {
                if(mkdir(dirPath, 0777) != 0)
                {
                    //创建失败
                    COMPRESS_LOG("mkdir fail : %s", dirPath);
                    return false;
                }
            }
        }
        
        lastPos = curPos;
        
    }
    
    return true;
}

void CompressRes::getRelativeDirPath(char *dirPath, char *subDirPath, char *outPath)
{
    char *startPos = strstr(dirPath, subDirPath);
    if(startPos != NULL)
    {
        sprintf(outPath, "%s", startPos+1);
    }
}

string CompressRes::encode(const unsigned char* Data, int DataLen, char *outByte)
{
    unsigned char *flagBit = (unsigned char *)malloc(sizeof(int));
    memcpy(flagBit, &m_nDecodeFlag, sizeof(int));
    int curFlagBit = 0;
    
    int curDataPos = 0;
    while(true)
    {
        outByte[curDataPos] = Data[curDataPos]^flagBit[curFlagBit];
        
        curDataPos ++;
        if (curDataPos >= DataLen)
        {
            break;
        }
        
        curFlagBit ++;
        if (curFlagBit >= sizeof(int))
        {
            curFlagBit = 0;
        }
    }
    
    return "";
}

#endif

