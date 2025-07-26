
#include "../lib/SDcardcontrol.h"

String savedDir = "";
String saveFile = "";
String saveReadFileFront = "";
String saveReadFileBack = "";
String indexesName;  // 创建索引文件存放路径+名称
String pagePathname; // 书页位置

void LittleFSinit()
{
    if (SD.begin(true)) // true为初始化失败进行一次格式化
    {
        Serial.println("闪存文件启动成功!");
    }
    else
        Serial.println("闪存启动失败");
}

int listDir(fs::FS &fs, const char *dirname, uint8_t levels, uint8_t gettarget)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        // Serial.println("Failed to open directory");
        return 999;
    }
    if (!root.isDirectory())
    {
        // Serial.println("Not a directory");
        return 666;
    }

    int FILE_NUMBER = 0;
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1, 0);
            }
            if (gettarget == 1)
            {
                savedDir = savedDir + ",,," + file.name();
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
            if (gettarget == 1)
            {
                saveFile = saveFile + ",,," + file.name();
            }

            FILE_NUMBER++;
        }
        file = root.openNextFile();
    }
    return FILE_NUMBER;
}

void createDir(fs::FS &fs, const char *path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path))
    {
        Serial.println("Dir created");
    }
    else
    {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char *path)
{
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    uint8_t i = 0;
    Serial.print("Read from file: ");
    while (file.available())
    {
        i++;
        char c = file.read();
        Serial.write(file.read());
        if (i < 8)
        {
            saveReadFileFront += c;
        }
        else
        {
            saveReadFileBack += c;
        }
    }
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2))
    {
        Serial.println("File renamed");
    }
    else
    {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path))
    {
        Serial.println("File deleted");
    }
    else
    {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file)
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    }
    else
    {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++)
    {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void PackFileName()
{
}

String GetSavedDirFile(uint8_t i)
{
    if (i == 0)
    {
        return saveFile;
    }
    else
    {
        return savedDir;
    }
    return "unknown";
}

// String GetSavedPage(uint8_t i) // 获取存储的1文件大小，2页数
// {
//     readFile(LittleFS, pagePathname.c_str());
//     if (i == 0)
//     {
//         String a = saveReadFileFront;
//         saveReadFileFront = "";
//         return a;
//     }
//     else
//     {
//         String a = saveReadFileBack;
//         saveReadFileBack = "";
//         return a;
//     }
//     return "unknown";
// }
// 清空变量
void ClearSavedData()
{
    savedDir = "";
    saveFile = "";
}

// 获取ascii字符的长度
int8_t getCharLength(char zf)
{
    if (zf == 0x20)
        return 4; // 空格
    else if (zf == '!')
        return 4;
    else if (zf == '"')
        return 5;
    else if (zf == '#')
        return 5;
    else if (zf == '$')
        return 6;
    else if (zf == '%')
        return 7;
    else if (zf == '&')
        return 7;
    else if (zf == '\'')
        return 3;
    else if (zf == '(')
        return 5;
    else if (zf == ')')
        return 5;
    else if (zf == '*')
        return 7;
    else if (zf == '+')
        return 7;
    else if (zf == ',')
        return 3;
    else if (zf == '.')
        return 3;

    else if (zf == '1')
        return 5;
    else if (zf == ':')
        return 4;
    else if (zf == ';')
        return 4;
    else if (zf == '@')
        return 9;

    else if (zf == 'A')
        return 8;
    else if (zf == 'D')
        return 7;
    else if (zf == 'G')
        return 7;
    else if (zf == 'H')
        return 7;
    else if (zf == 'I')
        return 3;
    else if (zf == 'J')
        return 3;
    else if (zf == 'M')
        return 8;
    else if (zf == 'N')
        return 7;
    else if (zf == 'O')
        return 7;
    else if (zf == 'Q')
        return 7;
    else if (zf == 'T')
        return 7;
    else if (zf == 'U')
        return 7;
    else if (zf == 'V')
        return 7;
    else if (zf == 'W')
        return 11;
    else if (zf == 'X')
        return 7;
    else if (zf == 'Y')
        return 7;
    else if (zf == 'Z')
        return 7;

    else if (zf == '[')
        return 5;
    else if (zf == ']')
        return 5;
    else if (zf == '`')
        return 5;

    else if (zf == 'c')
        return 5;
    else if (zf == 'f')
        return 5;
    else if (zf == 'i')
        return 1;
    else if (zf == 'j')
        return 2;
    else if (zf == 'k')
        return 5;
    else if (zf == 'l')
        return 2;
    else if (zf == 'm')
        return 9;
    else if (zf == 'o')
        return 7;
    else if (zf == 'r')
        return 4;
    else if (zf == 's')
        return 5;
    else if (zf == 't')
        return 4;
    else if (zf == 'v')
        return 7;
    else if (zf == 'w')
        return 9;
    else if (zf == 'x')
        return 5;
    else if (zf == 'y')
        return 7;
    else if (zf == 'z')
        return 5;

    else if (zf == '{')
        return 5;
    else if (zf == '|')
        return 4;
    else if (zf == '}')
        return 5;

    else if ((zf >= 0 && zf <= 31) || zf == 127)
        return -1; // 没有实际显示功能的字符

    else
        return 6;
}

// 编码TXT
void RECodeTXT(fs::FS &fs, const char *path, const char *bookSUOYIN)
{
    File file = fs.open(path, "r");

    File indexesFile;                 // 本次打开的索引文件系统对象
    indexesName = bookSUOYIN;          // 传入主索引文件地址
    String txtName[3] = {}; // 存储书本的名字,最多支持3本
    uint8_t txtNum = 0;     // 本次要打开的txt文件序号 0或1或2

    bool IS_NEXTSITE = false;
    Serial.printf("Reading file: %s\n", path);

    String txt[22 + 1] = {}; // 0-7行为一页 共8行
    int8_t line = 0;        // 当前行
    char c;                 // 中间数据
    uint16_t en_count = 0;  // 统计ascii和ascii扩展字符 1-2个字节
    uint16_t ch_count = 0;  // 统计中文等 3个字节的字符
    uint8_t line_old = 0;   // 记录旧行位置
    boolean hskgState = 0;  // 行首4个空格检测 0-检测过 1-未检测

    uint32_t pageCount = 1;              // 页数计数
    boolean line0_state = 1;             // 每页页首记录状态位
    uint32_t yswz_count = 0;             // 待写入文件统计
    String yswz_str = "";                // 待写入的文件
    uint32_t txtTotalSize = file.size(); // 记录该TXT文件的大小，插入到索引的倒数14-8位

    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    while (file.available())
    {
        // ESP.wdtFeed();       // 喂狗

        if (line_old != line) // 行首4个空格检测状态重置
        {
            line_old = line;
            hskgState = 1; // 新行需要检测空格
        }

        if (line0_state == 1 && line == 0 && pageCount > 1)
        {
            line0_state = 0;
            uint32_t yswz_uint32 = file.position(); // 获取当前位置 yswz=页数位置
            // 页数位置编码处理
            if (yswz_uint32 >= 1000000)
                yswz_str += String(yswz_uint32);
            else if (yswz_uint32 >= 100000)
                yswz_str += "0" + String(yswz_uint32);
            else if (yswz_uint32 >= 10000)
                yswz_str += "00" + String(yswz_uint32);
            else if (yswz_uint32 >= 1000)
                yswz_str += "000" + String(yswz_uint32);
            else if (yswz_uint32 >= 100)
                yswz_str += "0000" + String(yswz_uint32);
            else if (yswz_uint32 >= 10)
                yswz_str += "00000" + String(yswz_uint32);
            else
                yswz_str += "000000" + String(yswz_uint32);
            yswz_count++;
            if (yswz_count == 1000) // 每500页控制屏幕显示一下当前进度
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                indexesFile = fs.open(indexesName, "a"); // 在索引文件末尾追加内容
                indexesFile.print(yswz_str);                   // 将待写入的缓存 写入索引文件中
                indexesFile.close();
                ///////// ////////////////////////////////////////////////////////////////////////////////
                // 自己的方式实现功能
                //appendFile(SD, bookSUOYIN, yswz_str.c_str()); // 在后面增加内容yswz_str

                yswz_str = "";  // 待写入文件清空
                yswz_count = 0; // 待写入计数清空

                // 计算剩余量,进度条
                uint32_t shengyu_int = txtTotalSize - file.available();
                float shengyu_float = (float(shengyu_int) / float(txtTotalSize)) * 100.0;
                // display_partialLine(3, String(shengyu_float) + "%");
                Serial.println(shengyu_float);
                Serial.println("写入索引文件");
            }
            // Serial.print("第"); Serial.print(pageCount); Serial.print("页，页首位置："); Serial.println(yswz_uint32);
        }

        c = file.read();               // 读取一个字节
        while (c == '\n' && line <= 21) // 检查换行符,并将多个连续空白的换行合并成一个
        {
            // 检测到首行并且为空白则不需要插入换行
            if (line == 0) // 等于首行，并且首行不为空，才插入换行
            {
                if (txt[line].length() > 0)
                    line++; // 换行
                else
                    txt[line].clear();
            }
            else // 非首行的换行检测
            {
                // 连续空白的换行合并成一个
                if (txt[line].length() > 0)
                    line++;
                else if (txt[line].length() == 0 && txt[line - 1].length() > 0)
                    line++;
                /*else if (txt[line].length() == 1 && txt[line - 1].length() == 1) hh = 0;*/
            }
            if (line <= 21)
                c = file.read();
            en_count = 0;
            ch_count = 0;
        }
        if (c == '\t') // 检查水平制表符 tab
        {
            if (txt[line].length() == 0)
                txt[line] += "    "; // 行首的一个水平制表符 替换成4个空格
            else
                txt[line] += "       "; // 非行首的一个水平制表符 替换成7个空格
        }
        else if ((c >= 0 && c <= 31) || c == 127) // 检查没有实际显示功能的字符
        {
            // ESP.wdtFeed();  // 喂狗
        }
        else
        {
            txt[line] += c;
        }

        // 检查字符的格式 + 数据处理 + 长度计算
        boolean asciiState = 0;
        byte a = B11100000;
        byte b = c & a;

        if (b == B11100000) // 中文等 3个字节
        {
            ch_count++;
            c = file.read();
            txt[line] += c;
            c = file.read();
            txt[line] += c;
        }
        else if (b == B11000000) // ascii扩展 2个字节
        {
            en_count += 14;
            c = file.read();
            txt[line] += c;
        }
        else if (c == '\t') // 水平制表符，代替两个中文位置，14*2
        {
            if (txt[line] == "    ")
                en_count += 20; // 行首，因为后面会检测4个空格再加8所以这里是20
            else
                en_count += 28; // 非行首
        }
        else if (c >= 0 && c <= 199)
        {
            en_count += getCharLength(c) + 1; // getCharLength=获取ascii字符的像素长度
            asciiState = 1;
        }

        uint16_t StringLength = en_count + (ch_count * 16); // 一个中文14个像素长度

        if (StringLength >= 204 && hskgState) // 检测到行首的4个空格预计的长度再加长一点
        {
            if (txt[line][0] == ' ' && txt[line][1] == ' ' &&
                txt[line][2] == ' ' && txt[line][3] == ' ')
            {
                en_count += 8;
            }
            hskgState = 0;
        }

        if (StringLength >= 227) // 283个像素检查是否已填满屏幕 ，填满一行
        {
            if (asciiState == 0)
            {
                line++;
                en_count = 0;
                ch_count = 0;
            }
            else if (StringLength >= 230)
            {
                char t = file.read();
                file.seek(-1, SeekCur); // 往回移
                int8_t cz = 294 - StringLength;
                int8_t t_length = getCharLength(t);
                byte a = B11100000;
                byte b = t & a;
                if (b == B11100000 || b == B11000000) // 中文 ascii扩展
                {
                    line++;
                    en_count = 0;
                    ch_count = 0;
                }
                else if (t_length > cz)
                {
                    line++;
                    en_count = 0;
                    ch_count = 0;
                }
            }
        }
        if (line == 22)
        {
            line0_state = 1;
            pageCount++;
            line = 0;
            en_count = 0;
            ch_count = 0;
            for (uint8_t i = 0; i < 23; i++)
                txt[i].clear();
        }
    }

    // 剩余的字节写入索引文件，并在末尾加入文件大小校验位14-8 页数记录位7-1
    uint32_t size_uint32 = txtTotalSize; // 获取当前TXT文件的大小
    String size_str = "";
    // TXT文件大小编码处理
    if (size_uint32 >= 1000000)
        size_str += String(size_uint32);
    else if (size_uint32 >= 100000)
        size_str += String("0") + String(size_uint32);
    else if (size_uint32 >= 10000)
        size_str += String("00") + String(size_uint32);
    else if (size_uint32 >= 1000)
        size_str += String("000") + String(size_uint32);
    else if (size_uint32 >= 100)
        size_str += String("0000") + String(size_uint32);
    else if (size_uint32 >= 10)
        size_str += String("00000") + String(size_uint32);
    else
        size_str += String("000000") + String(size_uint32);

    if (yswz_count != 0) // 还有剩余页数就在末尾加入 剩余的页数+文件大小位+当前位置位（初始0）
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        indexesFile = fs.open(indexesName, "a");
        indexesFile.print(yswz_str + size_str + "0000000");
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // appendFile(SD, bookSUOYIN, yswz_str.c_str()); // 在后面增加内容yswz_str
        // appendFile(SD, bookSUOYIN, size_str.c_str());
        // appendFile(SD, bookSUOYIN, "0000000");
        /////////////////////////////////////////////////////////
        // appendFile(SD, pagePathname.c_str(), yswz_str.c_str()); // 在后面增加内容yswz_str
        // appendFile(SD, pagePathname.c_str(), size_str.c_str());
        // appendFile(SD, pagePathname.c_str(), "0000000");
    }
    else // 没有剩余页数了就在末尾加入文件大小位+当前位置位
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        indexesFile = fs.open(indexesName, "a");
        indexesFile.print(size_str + "0000000");
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // appendFile(SD, bookSUOYIN, "0000000");
        /////////////////////////////////////////////////////////////
        //appendFile(SD, pagePathname.c_str(), "0000000");
    }
    //indexesFile = SD.open(indexesName, "a");
    uint32_t indexes_size = indexesFile.size();
    Serial.print("索引文件大小：");
    Serial.println(indexes_size);
    Serial.print("yswz_count：");
    Serial.println(yswz_count);
    Serial.print("pageCount：");
    Serial.println(pageCount);

    // 校验索引是否正确建立
    // 算法：一页为7个字节（从第二页开始记录所以要总页数-1），加上文件大小位7个字节，加上当前页数位7个字节
    // 所以为：7*((总页数-1)+1+1))
    if (indexes_size == 7 * ((pageCount - 1) + 1 + 1))
    {
        Serial.println("校验通过，索引文件有效");
        // display_partialLine(3, "100.00%");
        // display_partialLine(4, "校验通过，索引文件有效");
    }
    else
    {
        Serial.println("校验失败，索引文件无效，请重新创建");
        // display_partialLine(7, "索引文件创建失败，校验失败或空间不足");
        //fs.remove(indexesName); // 删除无效的索引
        // deleteFile(SD, bookSUOYIN);
        // display_bitmap_sleep(" "); //休眠
    }

    //   display_partialLine(5, "索引文件大小：" + byteConversion(indexes_size));

    indexesFile.close();

    yswz_str = "";
    yswz_count = 0;

    file.close();
    // file = SD.open(txtName[txtNum], "r");

    // uint32_t time3 = millis() - time1;
    Serial.print("计算完毕：");
    Serial.print(pageCount);
    Serial.println("页");
    // Serial.print("耗时：");
    // Serial.print(time3);
    // Serial.println("毫秒");
    //   display_partialLine(6, "耗时：" + String(float(time3) / 1000.0) + "秒");
    //delay(500);
    line = 0;
    en_count = 0;
    ch_count = 0;
    // for (uint8_t i = 0; i < 9; i++)
    //     txt[i].clear();
}

//我的编码TXT方案->用于生成小说目录
void DECodeTXT371(fs::FS &fs, const char *path, const char *bookSUOYIN){
    File file = fs.open(path);

    File indexesFile;                 // 本次打开的索引文件系统对象
    indexesName = bookSUOYIN;          // 传入主索引文件地址

    Serial.printf("Reading file: %s\n", path);

    String txt[8 + 1] = {}; // 0-7行为一页 共8行
    int8_t line = 0;        // 当前行
    char c;                 // 中间数据
    uint16_t en_count = 0;  // 统计ascii和ascii扩展字符 1-2个字节
    uint16_t ch_count = 0;  // 统计中文等 3个字节的字符
    uint8_t line_old = 0;   // 记录旧行位置
    boolean hskgState = 0;  // 行首4个空格检测 0-检测过 1-未检测

    uint32_t pageCount = 1;              // 页数计数
    boolean line0_state = 1;             // 每页页首记录状态位
    uint32_t yswz_count = 0;             // 待写入文件统计
    String yswz_str = "";                // 待写入的文件
    uint32_t txtTotalSize = file.size(); // 记录该TXT文件的大小，插入到索引的倒数14-8位

    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    while (file.available()){

    }
}