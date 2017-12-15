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

const char *TXT_SUFFIX_LIST[] = {"plist", "lua", ""};

CompressRes::CompressRes()
{
}

void CompressRes::addCompressDir(string dirName)
{
    m_dirList.push_back(dirName);
}

bool CompressRes::Compress()
{
    string _rootPath = getcwd(NULL, NULL);
    string resCompressPath = _rootPath + "/res_compress";
    
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
    FILE *decompressFile = fopen(compressPath, "r");
    
    while(true)
    {
        int pathLen = 0;
        if (fread(&pathLen, sizeof(unsigned int), 1, decompressFile) == 0)
        {
            //不成功或读到文件末尾
            break;
        }
        
        char *filePath = (char *)malloc(pathLen);
        if (fread(filePath, pathLen, 1, decompressFile) == 0)
        {
            break;
        }
        COMPRESS_LOG("CompressRes::DeCompress = %s\n", filePath);
        
        int fileConentLen = 0;
        if (fread(&fileConentLen, sizeof(unsigned int), 1, decompressFile) == 0)
        {
            break;
        }
        
        char *fileContentBuf = (char*)malloc(fileConentLen);
        if (fread(fileContentBuf, fileConentLen, 1, decompressFile) == 0)
        {
            break;
        }
        
        char fileWritePath[1024] = {0};
        sprintf(fileWritePath, "%s%s", destWritePath, filePath);
        FILE *newFile;
        createFile(fileWritePath, &newFile);
        fwrite(fileContentBuf, fileConentLen, 1, newFile);
        fclose(newFile);
    }
    
    fclose(decompressFile);
    
    return true;
}

bool  CompressRes::DeCompress(unsigned char *fileContent, long fileSize, const char *destWritePath)
{
    long curSize = 0;
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
        
        char fileWritePath[1024] = { 0 };
        sprintf(fileWritePath, "%s%s", destWritePath, filePath);
        FILE *newFile;
        createFile(fileWritePath, &newFile);
        fwrite(fileContentBuf, fileConentLen, 1, newFile);
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
            
            COMPRESS_LOG("CompressRes  %s\n", filePath);
            unsigned int pathLen = strlen(filePath) + 1;  // +1表示写入末尾的\0
            if (fwrite(&pathLen, sizeof(unsigned int), 1, destCompressFile) != 1) // 写入文件路径长度
            {
                COMPRESS_LOG("pathLen write error");
                break;
            }
            
            if (fwrite(filePath, 1, pathLen, destCompressFile) != pathLen)  // 写入文件路径
            {
                COMPRESS_LOG("tempPath write error");
                break;
            }
            
            unsigned int fileContentLen = 0;
            char *contentBuf = NULL;
            if(isTxtFileType(dirp->d_name)) // txt type
            {
                contentBuf = readTxtFile(tempPath, fileContentLen);
            }
            else
            {
                contentBuf = readBinaryFile(tempPath, fileContentLen);
            }
            
            // 写入文件数据大小
            if (fwrite(&fileContentLen, sizeof(unsigned int), 1, destCompressFile) != 1)
            {
                COMPRESS_LOG("fileContentLen write error");
            }
            
            // 写入文件数据
            if (fwrite(contentBuf, fileContentLen, 1, destCompressFile) != 1)
            {
                COMPRESS_LOG("contentBuf write error");
            }
            
            free(contentBuf);
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
    unsigned int pathLen = strlen(fileName) + 1;  // +1表示写入末尾的\0
    if (fwrite(&pathLen, sizeof(unsigned int), 1, destCompressFile) != 1) // 写入文件路径长度
    {
        COMPRESS_LOG("pathLen write error");
    }
    
    if (fwrite(fileName, 1, pathLen, destCompressFile) != pathLen)  // 写入文件路径
    {
        COMPRESS_LOG("tempPath write error");
    }
    COMPRESS_LOG("CompressRes  %s\n", fileName);
    
    unsigned int fileContentLen = 0;
    char *contentBuf = NULL;
    if(isTxtFileType(fileName)) // txt type
    {
        contentBuf = readTxtFile((char*)filePath, fileContentLen);
    }
    else
    {
        contentBuf = readBinaryFile((char*)filePath, fileContentLen);
    }
    
    // 写入文件数据大小
    if (fwrite(&fileContentLen, sizeof(unsigned int), 1, destCompressFile) != 1)
    {
        COMPRESS_LOG("fileContentLen write error");
    }
    
    // 写入文件数据
    if (fwrite(contentBuf, fileContentLen, 1, destCompressFile) != 1)
    {
        COMPRESS_LOG("contentBuf write error");
    }
    
    free(contentBuf);
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

char *CompressRes::readTxtFile(char *fname, unsigned int &size)
{
    FILE *fp;
    char *str;
    char txt[1000];
    int filesize;
    if ((fp=fopen(fname,"r"))==NULL){
        COMPRESS_LOG("打开文件%s错误\n",fname);
        return NULL;
    }
    
    fseek(fp,0,SEEK_END);
    
    filesize = ftell(fp);
    size = filesize;
    str=(char *)malloc(filesize);
    str[0]=0;
    
    rewind(fp);
    while((fgets(txt,1000,fp))!=NULL){
        strcat(str,txt);
    }
    fclose(fp);
    return str;
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

void CompressRes::getFileSuffix(const char *filename, char *out)
{
    char *dotPos = strrchr((char*)filename, '.');
    
    if(dotPos != NULL)
    {
        sprintf(out, "%s", dotPos+1);
    }
}

bool CompressRes::isTxtFileType(const char *filename)
{
    bool isTxtFile = false;
    
    char suffixBuf[1024] = {0};
    getFileSuffix(filename, suffixBuf);
    
    int suffixIndex = 0;
    while(true)
    {
        const char *curSuffix = TXT_SUFFIX_LIST[suffixIndex];
        if(curSuffix == "")
        {
            break;
        }
        
        if(strcmp(curSuffix, suffixBuf) == 0)
        {
            isTxtFile = true;
            break;
        }
        
        suffixIndex ++;
    }
    
    return isTxtFile;
}

void CompressRes::getRelativeDirPath(char *dirPath, char *subDirPath, char *outPath)
{
    char *startPos = strstr(dirPath, subDirPath);
    if(startPos != NULL)
    {
        sprintf(outPath, "%s", startPos+1);
    }
}

#endif

