#include "kvfs.h"

//All path passed to kvfs_functions are relative. So call this function to get fullpath.
static char* kvfs_getfullpath(const char *path)
{
    static char fullpath[64];
    strcpy(fullpath, KVFS_DATA->rootdir);
    char* root="/";
    strncat(fullpath, root, strlen(root)+1);
    if (strcmp(path, str2md5(root, strlen(root)))!=0)
    {
	strncat(fullpath, path, strlen(path)+1);
    }
    log_msg ("Fullpath returned: %s\n", fullpath);
    return fullpath;
}

int kvfs_getattr_impl(const char *path, struct stat *statbuf)
{
    if (lstat(kvfs_getfullpath(path), statbuf) < 0)
	return -errno;
    else
	return 0;
}




int kvfs_readlink_impl(const char *path, char *link, size_t size)
{
    //reserving one space for ‘\0’ in case buff is too small
    int result = readlink(kvfs_getfullpath(path), link, size-1);
    if(result < 0)
	result = -errno;
    else
  	link[result] = '\0';
   return result;
}


int kvfs_mknod_impl(const char *path, mode_t mode, dev_t dev)
{
   if (mknod(kvfs_getfullpath(path), mode, dev) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_mkdir_impl(const char *path, mode_t mode)
{
    if (mkdir(kvfs_getfullpath(path), mode) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_unlink_impl(const char *path)
{
    if (unlink(kvfs_getfullpath(path)) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_rmdir_impl(const char *path)
{
    if (rmdir(kvfs_getfullpath(path)) < 0)
    	return -errno;
    else
	return 0;
}

int kvfs_symlink_impl(const char *path, const char *link)
{
    if(symlink(path, kvfs_getfullpath(link))<0)
	return -errno;
    else
	return 0;
}

int kvfs_rename_impl(const char *path, const char *newpath)
{
     char fullpath[64];
    strcpy(fullpath, kvfs_getfullpath(path));
    char newfullpath[64];
    strcpy(newfullpath, kvfs_getfullpath(newpath));
    char *addfullpath=fullpath;
    char *addnewfullpath=newfullpath;
    log_msg ("Rename. Path: %s. NewPath: %s\n", path, newpath);
    if (link(addfullpath, addnewfullpath) < 0)
        return -errno;
    if(unlink(addfullpath) < 0)
        return -errno;
    return 0;
}

int kvfs_link_impl(const char *path, const char *newpath)
{
    char fullpath[64];
    strcpy(fullpath, kvfs_getfullpath(path));
    char newfullpath[64];
    strcpy(newfullpath, kvfs_getfullpath(newpath));
    char *addfullpath=fullpath;
    char *addnewfullpath=newfullpath;
    log_msg ("Link. Path: %s. NewPath: %s\n", path, newpath);
    if (link(addfullpath, addnewfullpath) < 0)
        return -errno;
    else
        return 0;
}

int kvfs_chmod_impl(const char *path, mode_t mode)
{
    if (chmod(kvfs_getfullpath(path), mode) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_chown_impl(const char *path, uid_t uid, gid_t gid)
{
    //lchown() is like chown(), but does not dereference symbolic links.
    if (lchown(kvfs_getfullpath(path), uid, gid) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_truncate_impl(const char *path, off_t newsize)
{
   if (truncate(kvfs_getfullpath(path), newsize) < 0)
		return -errno;
  else
	return 0;

}

int kvfs_utime_impl(const char *path, struct utimbuf *ubuf)
{
    if(utime(kvfs_getfullpath(path), ubuf) < 0)
        return -errno;
    else
	return 0;
}

int kvfs_open_impl(const char *path, struct fuse_file_info *fi)
{
    int flag=fi->flags;
    int filedescriptor= open(kvfs_getfullpath(path), flag);
    if (filedescriptor<0)
	return -errno;
    fi->fh = filedescriptor;
    return 0;
}

int kvfs_read_impl(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
   int r = pread(fi->fh, buf, size, offset);
   if (r < 0)
   	 return -errno;
    else
	return r;
}

int kvfs_write_impl(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int r = pwrite(fi->fh, buf, size, offset);
   if (r < 0)
   	 return -errno;
    else
        	return r;
}

int kvfs_statfs_impl(const char *path, struct statvfs *statv)
{
    if (statvfs(kvfs_getfullpath(path), statv) < 0)
	return -errno;
    else
	return 0;
}

//This is called after opening and closing file. So dummy implementation.
int kvfs_flush_impl(const char *path, struct fuse_file_info *fi)
{
   return 0;
}

int kvfs_release_impl(const char *path, struct fuse_file_info *fi)
{
     if (close(fi->fh) < 0)
   	 return -errno;
    else
   	 return 0;
}

int kvfs_fsync_impl(const char *path, int datasync, struct fuse_file_info *fi)
{
 	 if(datasync!=0){
		if(fdatasync(fi->fh) < 0)
			return -errno;
		else return 0;
	}
	else{
		if(fsync(fi->fh)<0)
			return -errno;
   		else
			return 0;
	}
}

#ifdef HAVE_SYS_XATTR_H
int kvfs_setxattr_impl(const char *path, const char *name, const char *value, size_t size, int flags)
{
    return -1;
}

int kvfs_getxattr_impl(const char *path, const char *name, char *value, size_t size)
{
    return -1;
}

int kvfs_listxattr_impl(const char *path, char *list, size_t size)
{
    return -1;
}

int kvfs_removexattr_impl(const char *path, const char *name)
{
    return -1;
}
#endif

int kvfs_opendir_impl(const char *path, struct fuse_file_info *fi)
{
    //add check as mentioned above.
    DIR *dirpoint = opendir(kvfs_getfullpath(path));
    if(dirpoint == NULL)
        return -errno;
    //check in vcl for unintptr
    //check if returning fd is necessary or not.
    fi->fh = (uintptr_t) dirpoint;
    return 0;
}

int kvfs_readdir_impl(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    errno=0;
    DIR *dirpoint=(DIR *) (uintptr_t) fi->fh;
    struct dirent *de = readdir(dirpoint);
    if (de == NULL && errno!=0) {
        return -errno;
    }
    do {
        if (filler(buf, de->d_name, NULL, 0) != 0) {
            return -ENOMEM;
        }
    } while ((de = readdir(dirpoint)) != NULL);
    return 0;
}

int kvfs_releasedir_impl(const char *path, struct fuse_file_info *fi)
{
    if (closedir((DIR *) (uintptr_t) fi->fh) < 0)
   	 return -errno;
    else
   	 return 0;
}

int kvfs_fsyncdir_impl(const char *path, int datasync, struct fuse_file_info *fi)
{
    	 if(datasync!=0){
		if(fdatasync(fi->fh) < 0)
			return -errno;
		else return 0;
	}
	else{
		if(fsync(fi->fh)<0)
			return -errno;
   		else
			return 0;
	}

}

int kvfs_access_impl(const char *path, int mask)
{
    if (access(kvfs_getfullpath(path), mask) < 0)
	return -errno;
    else
	return 0;
}

int kvfs_ftruncate_impl(const char *path, off_t offset, struct fuse_file_info *fi)
{

    if(ftruncate(fi->fh, offset) < 0)
	return -errno;
    else return 0;
}

int kvfs_fgetattr_impl(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    	if(fstat(fi->fh, statbuf)<0)
	    return -errno;
	else
	    return 0;
}


