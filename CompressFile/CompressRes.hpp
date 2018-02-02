//
//  CompressRes.hpp
//  mahjonghubei
//
//  Created by jlb on 2017/12/4.
//

/*
 * 压缩文件具体用法：
 * printf("start compress...\n");
 * CompressRes tt;
 *     
 *  printf("enter compress file name: ");
 *  char buff[1024] = {0};
 *  
 *  tt.setCompressFileName(buff);
 *  
 *  printf("enter decode flag num: ");
 *  int flagNum = 0;
 *  scanf("%d", &flagNum);  //5 编码值
 *  tt.setDecodeFlag(flagNum);
 *  
 *  while(true)
 *  {
 *      printf("enter dir name: ");
 *      char buff[1024] = {0};
 *      scanf("%s", buff);
 *      
 *      if(strcmp(buff, ".") == 0){
 *          break;
 *      }
 *      
 *      tt.addCompressDir(buff);
 *  }
 *  tt.Compress();

*/

/*
 * 解压缩具体方法
 * #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
 * bool firstExecute = UserDefault::getInstance()->getBoolForKey("firstExecute", false);
 * if (firstExecute == false)
 * {
 *     string compressPath = FileUtils::getInstance()->fullPathForFilename("res_compress");
 *     cocos2d::log("start compressPath");
 *     CompressRes rtt;
 *	   rtt.setDecodeFlag(5);
 *     rtt.DeCompress(compressPath.c_str(), FileUtils::getInstance()->getWritablePath().c_str());
 *     UserDefault::getInstance()->setBoolForKey("firstExecute", true);
 * }
 * #elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
 * bool firstExecute = UserDefault::getInstance()->getBoolForKey("firstExecute", false);
 * if (firstExecute == false)
 * {
 *     // android的assets文件夹无法用FILE打开
 *     long fileSize = 0;
 *     unsigned char *buf = FileUtils::getInstance()->getFileData("res_compress", "r", &fileSize);
 *     cocos2d::log("start compressPath");
 *     CompressRes rtt;
 *     rtt.setDecodeFlag(5);
 *     rtt.DeCompress(buf, fileSize, FileUtils::getInstance()->getWritablePath().c_str());
 *     UserDefault::getInstance()->setBoolForKey("firstExecute", true);
 * }
 * #endif
*/

#if !defined(_WIN32)

#ifndef CompressRes_hpp
#define CompressRes_hpp

#include <string>
#include <vector>

#if defined(ANDROID)

#include "android/log.h"

#define  LOG_TAG    "CompressRes-log"
#define  COMPRESS_LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#elif defined(__APPLE__)

#define  COMPRESS_LOG(...)  printf(__VA_ARGS__)

#endif

using namespace std;

class CompressRes
{
public:
    CompressRes();
    
    /*
     * 压缩文件
     * 压缩后文件格式为：
     * (1).4个字节,路径字符串的长度
     * (2).路径字符串
     * (3).4个字节,文件内容大小
     * (4).具体的文件内容
     */
    bool Compress();
    
    /*
     * 解压缩文件
     * eg.DeCompress("/var/containers/Bundle/compressFile", "usr/lzc/Documents/")
     */
    bool DeCompress(const char *compressPath, const char *destWritePath);
    bool DeCompress(unsigned char *fileContent, long fileSize, const char *destWritePath);
    
    /*
     * 添加需要压缩的目录
     */
    void addCompressDir(string dirName);
    
    void setCompressFileName(const char *fileName);
    std::string getCompressFileName();
    
    void setDecodeFlag(int decodeFlag);
    
private:
    /*
     * 递归压缩所有文件到destCompressFile
     */
    void compressDir(const char *sourceDirPath, FILE *destCompressFile, const char *sourceDirName);
    
    void compressFile(const char *filePath, FILE *destCompressFile, const char *fileName);
    
    /*
     * 读取二进制文件
     */
    char *readBinaryFile(char* fname, unsigned int &size);
    
    /*
     * 写数据到文件
     */
    bool writeData(char *filePath, FILE *destCompressFile, const char *fileName);
    
    /*
     * 创建文件，如果路径中存在文件夹没有创建则会创建文件夹直至完整路径生成
     */
    bool createFile(char *filepath, FILE **newFILE);
    
    /*
     * 获得文件的目录的子目录
     * eg: getRelativeDirPath("/lib/usr/lzc/itd/main.cpp", "/lzc/", out) -> "lzc/itd/main.cpp"
     */
    void getRelativeDirPath(char *dirPath, char *subDirPath, char *outPath);
    
    /*
	 * 编码:对数据进行异或编码
     */
    string encode(const unsigned char* Data,int DataLen,char *outByte);
    
private:
    std::vector<string> m_dirList;
    
    std::string m_compressFileName;
    
    int m_nDecodeFlag;
};

#endif /* CompressRes_hpp */

#endif

