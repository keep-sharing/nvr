/*
 * raid.h - manage Linux "md" devices aka RAID arrays.
 *
 *    Author: milesight hugo
 *    Date: 20160703
 */




#define MD_RESERVED_BYTES		(64 * 1024)
#define MD_RESERVED_SECTORS		(MD_RESERVED_BYTES / 512)
 //#define MD_RESERVED_BLOCKS        (MD_RESERVED_BYTES / BLOCK_SIZE)
 
#define MD_NEW_SIZE_SECTORS(x)		((x & ~(MD_RESERVED_SECTORS - 1)) - MD_RESERVED_SECTORS)
 //#define MD_NEW_SIZE_BLOCKS(x)     ((x & ~(MD_RESERVED_BLOCKS - 1)) - MD_RESERVED_BLOCKS)
 
#define     MD_SB_DISKS     (16)
 
#define MD_DISK_FAULTY		0 /* disk is faulty / operational */
#define MD_DISK_ACTIVE		1 /* disk is running but may not be in sync */
#define MD_DISK_SYNC		2 /* disk is in sync with the raid set */
#define MD_DISK_REMOVED		3 /* disk is in sync with the raid set */
#define MD_DISK_CLUSTER_ADD	4 /* Initiate a disk add across the cluster
                    * For clustered enviroments only.
                    */
#define MD_DISK_CANDIDATE	5 /* disk is added as spare (local) until confirmed
                    * For clustered enviroments only.
                    */
 
#define	MD_DISK_WRITEMOSTLY	9 /* disk is "write-mostly" is RAID1 config.
                    * read requests will only be sent here in
                    * dire need
                    */
 
#define MD_DISK_REPLACEMENT	17
#define MD_DISK_JOURNAL		18 /* disk is used as the write journal in RAID-5/6 */
 
#define MD_DISK_ROLE_SPARE	0xffff
#define MD_DISK_ROLE_FAULTY	0xfffe
#define MD_DISK_ROLE_JOURNAL	0xfffd
#define MD_DISK_ROLE_MAX	0xff00 /* max value of regular disk role */
 
 
 
#define	LEVEL_MULTIPATH		(-4)
#define	LEVEL_LINEAR		(-1)
#define	LEVEL_FAULTY		(-5)
 
#define GET_DISK_INFO		_IOR (MD_MAJOR, 0x12, mdu_disk_info_t)

#define	MD_MAJOR            9
 
#define GET_ARRAY_INFO		_IOR (MD_MAJOR, 0x11, mdu_array_info_t)
 
#define GET_BITMAP_FILE		_IOR (MD_MAJOR, 0x15, mdu_bitmap_file_t)


typedef struct mdu_array_info_s {
    /*
    * Generic constant information
    */
    int major_version;
    int minor_version;
    int patch_version;
    unsigned int ctime;
    int level;
    int size;
    int nr_disks;
    int raid_disks;
    int md_minor;
    int not_persistent;

    /*
    * Generic state information
    */
    unsigned int utime; /*  0 Superblock update time              */
    int state;      /*  1 State bits (clean, ...)             */
    int active_disks;   /*  2 Number of currently active disks        */
    int working_disks;  /*  3 Number of working disks             */
    int failed_disks;   /*  4 Number of failed disks              */
    int spare_disks;    /*  5 Number of spare disks           */

    /*
    * Personality information
    */
    int layout;     /*  0 the array's physical layout         */
    int chunk_size; /*  1 chunk size in bytes             */

} mdu_array_info_t;
 

 
struct spare_disk_info {
    int  port;
    char dev_path[DEV_NAME_LEN];
    unsigned long long size;
    //char mnt_path[PATH_NAME_MAX];
    int  state; //smartctl health status
};
 
struct spare_disk_list {
    struct spare_disk_info disk[MAX_DISK_LOCAL_NUM];
    int cnt;
};

typedef enum
{
    RAID_SUCCESS        = 0,   // raid op success
    RAID_FAILED         = -1,  // raid op faild 
    RAID_SIZE_SMALL_ERR = -2,  // no spare disk, because disk size is small
    RAID_DISK_LACK_ERR  = -3,  // disk is less, can not make raid
    RAID_DEL_SPARE_ERR  = -4,  // delete spare disk failed or no this spare disk.

}RAID_ERRNO_E;

typedef struct mdu_bitmap_file_s
{
    char pathname[4096];
} mdu_bitmap_file_t;
 
 
typedef struct mdu_disk_info_s {
    int number;
    int major;
    int minor;
    int raid_disk;
    int state;
} mdu_disk_info_t;
 

struct mdp_superblock_1 {
	/* constant array information - 128 bytes */
	__u32	magic;		/* MD_SB_MAGIC: 0xa92b4efc - little endian */
	__u32	major_version;	/* 1 */
	__u32	feature_map;	/* 0 for now */
	__u32	pad0;		/* always set to 0 when writing */

	__u8	set_uuid[16];	/* user-space generated. */
	char	set_name[32];	/* set and interpreted by user-space */

	__u64	ctime;		/* lo 40 bits are seconds, top 24 are microseconds or 0*/
	__u32	level;		/* -4 (multipath), -1 (linear), 0,1,4,5 */
	__u32	layout;		/* only for raid5 currently */
	__u64	size;		/* used size of component devices, in 512byte sectors */

	__u32	chunksize;	/* in 512byte sectors */
	__u32	raid_disks;
	__u32	bitmap_offset;	/* sectors after start of superblock that bitmap starts
				 * NOTE: signed, so bitmap can be before superblock
				 * only meaningful of feature_map[0] is set.
				 */

	/* These are only valid with feature bit '4' */
	__u32	new_level;	/* new level we are reshaping to		*/
	__u64	reshape_position;	/* next address in array-space for reshape */
	__u32	delta_disks;	/* change in number of raid_disks		*/
	__u32	new_layout;	/* new layout					*/
	__u32	new_chunk;	/* new chunk size (sectors)			*/
	__u32	new_offset;	/* signed number to add to data_offset in new
				 * layout.  0 == no-change.  This can be
				 * different on each device in the array.
				 */

	/* constant this-device information - 64 bytes */
	__u64	data_offset;	/* sector start of data, often 0 */
	__u64	data_size;	/* sectors in this device that can be used for data */
	__u64	super_offset;	/* sector start of this superblock */
	union {
		__u64	recovery_offset;/* sectors before this offset (from data_offset) have been recovered */
		__u64	journal_tail;/* journal tail of journal device (from data_offset) */
	};
	__u32	dev_number;	/* permanent identifier of this  device - not role in raid */
	__u32	cnt_corrected_read; /* number of read errors that were corrected by re-writing */
	__u8	device_uuid[16]; /* user-space setable, ignored by kernel */
	__u8    devflags;        /* per-device flags.  Only one defined...*/
#define WriteMostly1    1        /* mask for writemostly flag in above */
	/* bad block log.  If there are any bad blocks the feature flag is set.
	 * if offset and size are non-zero, that space is reserved and available.
	 */
	__u8	bblog_shift;	/* shift from sectors to block size for badblocklist */
	__u16	bblog_size;	/* number of sectors reserved for badblocklist */
	__u32	bblog_offset;	/* sector offset from superblock to bblog, signed */

	/* array state information - 64 bytes */
	__u64	utime;		/* 40 bits second, 24 btes microseconds */
	__u64	events;		/* incremented when superblock updated */
	__u64	resync_offset;	/* data before this offset (from data_offset) known to be in sync */
	__u32	sb_csum;	/* checksum upto dev_roles[max_dev] */
	__u32	max_dev;	/* size of dev_roles[] array to consider */
	__u8	pad3[64-32];	/* set to 0 when writing */

	/* device state information. Indexed by dev_number.
	 * 2 bytes per device
	 * Note there are no per-device state flags. State information is rolled
	 * into the 'roles' value.  If a device is spare or faulty, then it doesn't
	 * have a meaningful role.
	 */
	__u16	dev_roles[0];	/* role in array, or 0xffff for a spare, or 0xfffe for faulty */
};


#define MAX_SB_SIZE 4096
/* bitmap super size is 256, but we round up to a sector for alignment */
#define BM_SUPER_SIZE 512
#define MAX_DEVS ((int)(MAX_SB_SIZE - sizeof(struct mdp_superblock_1)) / 2)

#define SUPER1_SIZE	(MAX_SB_SIZE + BM_SUPER_SIZE \
			 + sizeof(struct misc_dev_info))

struct misc_dev_info {
	unsigned long long device_size;
};

#define _ROUND_UP(val, base)	(((val) + (base) - 1) & ~(base - 1))
#define ROUND_UP(val, base)	_ROUND_UP(val, (typeof(val))(base))
#define ROUND_UP_PTR(ptr, base)	((typeof(ptr)) \
				 (ROUND_UP((unsigned long)(ptr), base)))


struct disk_assemble_list {
    int port; ///< sata port number
    char dev_path[DEV_NAME_LEN]; ///< disk path
};
                 
struct assemble_info {
    struct disk_assemble_list list[MAX_DISK_LOCAL_NUM];
    int     cnt; ///< current hdd count
    __u8    uuid[16];   /* user-space generated. */
    __u8    md_port;    /* set and interpreted by user-space */
	int		normalCnt // raid status is normal need disk count
};



