# Trivia

Данный курсач реализует модуль безопасности ядра. Он следит за процессами (для которых он включен)
и смотрит чтобы они никуда не лазяли по типу вне директории, к сторонним айпишникам и портам,
к неразрешенным сокетам, пайпам, или обычным файлам.

Управление с помощью  sequence file.

```bash
echo "restrict 9999" > /proc/mylsm/mylsm
echo "allow_ip 9999 127.0.0.1:80" > /proc/mylsm/mylsm 
echo "allow_file 9999 /path/to/file regular" > /proc/mylsm/mylsm # regular unix file
echo "allow_file 9999 /path/to/file.sock unix_socket" > /proc/mylsm/mylsm
echo "allow_file 9999 /path/to/file.pipe pipe" > /proc/mylsm/mylsm
echo "allow_directory 9999 /path/to/directory" > /proc/mylsm/mylsm
cat /proc/mylsm/mylsm
echo "info_pids 0" > /proc/mylsm/mylsm
cat /proc/mylsm/mylsm
echo "info_restriction 9999" > /proc/mylsm/mylsm 
cat /proc/mylsm/mylsm
```

Terminal apps:
```bash
# ssh window 1
python ./python_tests/restrict_me.py

# ssh window 2
python ./python_tests/restricter.py
```

## Building and running

Способ описанный tut не предполагает загружаемый модуль. Он предполагает встроенный модуль.
В ядро. С перекомпиляцией ядра.

Идеологически они одинаковые только у этого модуля нет метода exit, и выгрузить его не получится.

1. Скачал Linux с Kernel Security Subsystem из git.kernel.org, следуя инструкции из [KernSec](https://kernsec.org/wiki/index.php/Kernel_Repository)
2. Сделал git checkout на next-testing (!!!!!), версия ядра 5.18. next-general у меня не компилировался.
3. Make kernel:
```bash
make x86_64_config
make kvm_guest.config
make -j12
```
4. Build virtual machine:

Последовал [этому видосу](https://www.youtube.com/watch?v=GeQZ2GKhfAE&feature=youtu.be)
Пришлось поставить пару пакетов и создать несколько файлов.

```bash
git clone https://github.com/bgmerrell/vkerndev
cd vkerndev
./make_vm.py
./run_vm.py --kernel ./path/to/linux/arch/x86/boot/bzImage --kernel-header ./path/to/linux/usr/include
```

## Разработка модуля ядра

Следую этому видосу [youtube](https://www.youtube.com/watch?v=Y0QZpan5LbU).

5. Сделал свою папку внутри `/linux/security`: mylsm

Создал Kconfig, makefile, my_secutiry_lsm.c

/linux/security/mylsm/Kconfig:

```makefile
config SECURITY_MYLSM
  bool "MYLSM support"
  depends on SECURITY
  default n
  help
    This is isolation software developed by Mikhail Scherbina
```

/linux/security/mylsm/Makefile:

```makefile
obj-$(CONFIG_SECURITY_MYLSM) := mylsm.o
mylsm-y:  mylsm.o
```

/linux/security/mylsm/mylsm.c:

```c
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/lsm_hooks.h>
#include <linux/kern_levels.h>
#include <linux/binfmts.h>

static int my_test_bprm_check_security(struct linux_binprm *bprm)
{
	printk(KERN_ERR "Hello from mylsm: %s\n", bprm->interp);
	return 0;
}

static struct security_hook_list my_test_hooks[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(bprm_check_security, my_test_bprm_check_security),
};

static int __init my_test_init(void)
{
	printk(KERN_ERR "mytest: We are going to do things\n");
	security_add_hooks(my_test_hooks, ARRAY_SIZE(my_test_hooks), "my_test");
	return 0;
}

DEFINE_LSM(mylsm) = {
	.name = "mylsm",
	.init = my_test_init,
};
```




Добавил в /linux/.config:
```
CONFIG_SECURITY_MYLSM=y
...
```


Добавил в /linux/security/Kconfig:
```
CONFIG_SECURITY_MYLSM=y
...
source "security/lockdown/Kconfig"
source "security/landlock/Kconfig"
source "security/mylsm/Kconfig"
```


Добавил в /linux/security/makefile:
```
obj-$(CONFIG_BPF_LSM)			+= bpf/
obj-$(CONFIG_SECURITY_LANDLOCK)		+= landlock/
obj-$(CONFIG_SECURITY_MYLSM)		+= mylsm/
```


```
make menuconfig

secutity options ->
    MY LSM support <- tick yes
    (большой лист с landlock,lockdown,yama,loadpin) <- добавил туда свой модуль mylsm
```

После этого надо бы перебилдить всё:

```
make -j12 bzImage
```

Теперь при запуски qemu с кернелом видны собщения из ядра.
