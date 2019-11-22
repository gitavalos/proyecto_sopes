#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include<linux/unistd.h>
//#include <asm/unistd.h>
//#include <sys/syscall.h>

MODULE_LICENSE("GPL");
/*
// IOCTL commands
#define IOCTL_PATCH_TABLE 0x00000001
#define IOCTL_FIX_TABLE 0x00000004
*/
//direccion de systable, se encuentra con el siguiente comando: cat /proc/kallsyms  | grep sys_call
// tomar la direccion de sys_call_table
//realizar cada vez que se reinicie el sistema puesto que la direccion cambia
#define dir_systable 0xffffffff95800200

unsigned long *sys_call_table;

//puntero de la funcion del sys_openat
//asmlinkage long (*real_open)(const char* __user, int, int);

struct filename *getname_filename;

asmlinkage long (*original_sys_unlink) (const char *pathname);

/*return -1. this will prevent any process from unlinking any file*/
asmlinkage long hacked_sys_unlink(const char __user *pathname)
{
	//getname_filename = getname(pathname);
	//getname_filename->name
	char * temp;
	strncpy_from_user(temp, pathname, 10);
    printk("RETENIDO: unlink( %s )\n", temp);
    //return original_sys_unlink(pathname);
	return -1;
}
        

//Reemplazando la llamada original con la llamada modificada
/*
asmlinkage long custom_open(const char* __user file_name, int flags, int mode)
{
	printk("INTERCEPTADO: open(\"%s\", %X, %X)\n", file_name,flags,mode);
	return real_open(file_name,flags,mode);	
}*/

/*
Modificando la pagina de la memoria para escritura
Esto es un poco riesgoso ya que se modifica el bit de proteccion a nivel de arquitectura
*/

int make_rw(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~ _PAGE_RW){
		pte->pte |= _PAGE_RW;
		printk("RW seteado\n");
	}
	return 0;
}

/* Protegiendo la pagina contra escritura*/
int make_ro(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	printk("RO seteado\n");
	return 0;
}

static int __init init_my_module(void)
{
	*(long *)&sys_call_table = dir_systable;
	//para este ejemplo se utiliza la llamada al sistema openat para abir archivos
	printk(KERN_INFO "Inside kernel space\n");
	//cambiando permisos de la pagina
	make_rw((unsigned long)sys_call_table);	
	original_sys_unlink = (void *)xchg(&sys_call_table[__NR_unlinkat],hacked_sys_unlink);
	make_ro((unsigned long)sys_call_table);
	printk("hizo el cambio de pagina \n");
	return 0;
}

static void __exit cleanup_my_module(void)
{
	
	//cambiando la direccion de memoria a modo de escritura
	make_rw((unsigned long)sys_call_table);
	
	xchg(&sys_call_table[__NR_unlinkat],original_sys_unlink);
	
	make_ro((unsigned long)sys_call_table);
	printk(KERN_INFO "Exiting kernel space\n");
	return;
}

module_init(init_my_module);
module_exit(cleanup_my_module);