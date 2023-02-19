#ifndef __MACRO__
    #define __MACRO__
    //虚拟磁盘空间设计
    #define BLOCK_NUMS 1024                             // 磁盘中的磁盘块数目
    #define BLOCK_SIZE 1024                             // 每个磁盘块大小(单位:Bytes)
    
    //磁盘块块内设计
    #define FAT_ITEM_SIZE 2                             // FAT表项大小(字节)
    #define FAT_ITEM_NUM 1024                           // FAT表项数
    #define FCB_SIZE 24                                 // FCB大小(字节)
    #define FCB_ITEM_NUM 42                             // FCB大小

    //预定义盘块各个部分位置
    #define FAT1_INIT_BLOCK 1                           // FAT1起始盘块位置(盘块号)
    #define FAT2_INIT_BLOCK 3                           // FAT2起始盘块位置(盘块号)
    #define ROOT_FCB_LOCATION 5                         // 根目录FCB位置(盘块号)
    #define DATA_INIT_BLOCK 6                           // 数据块起始位置(盘块号)
    #define FAT1_LOCATON (BLOCK_SIZE*FAT1_INIT_BLOCK)   // FAT1起始位置的字节数
    #define FAT2_LOCATON (BLOCK_SIZE*FAT2_INIT_BLOCK)   // FAT2起始位置的字节数
    #define DATA_LOCATION (BLOCK_SIZE*DATA_INIT_BLOCK)  // 数据块起始位置的字节数 
    
    //文件设计
    #define FILE_NAME_LEN 11                            // 文件名长度(字节)
    #define MAX_FD_NUM 10                               // 最大支持打开的文件标识符个数
    
    //FAT表项数据配置
    #define END_OF_FILE -1                              // 文件结尾盘块
    #define FREE 0                                      // 空闲盘块
    #define USED 1                                      // 分配给引导块/FAT的磁盘块
    #define FCB_BLOCK 2                                 // 存储文件控制块FCB的盘块
#endif