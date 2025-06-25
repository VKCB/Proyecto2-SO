#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "mrobot"
#define CLASS_NAME "mrobot_class"

static int major_number;
static struct class* mrobot_class = NULL;
static struct device* mrobot_device = NULL;
static struct cdev mrobot_cdev;

// Simulación de interacción física
static int position[2] = {0, 0};

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "mrobot: Dispositivo abierto\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "mrobot: Dispositivo cerrado\n");
    return 0;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    if (len == sizeof(int) * 2) {
        if (copy_from_user(position, buffer, len) == 0) {
            printk(KERN_INFO "mrobot: Movimiento recibido - X: %d, Y: %d\n", position[0], position[1]);
            return len;
        }
    } else if (len == 1) {
        char cmd;
        if (copy_from_user(&cmd, buffer, 1) == 0 && cmd == 'P') {
            printk(KERN_INFO "mrobot: Comando de presión recibido (P)\n");
            return 1;
        }
    }

    printk(KERN_WARNING "mrobot: Error en los datos recibidos\n");
    return -EINVAL;
}

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .write = dev_write,
};

static int __init mrobot_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "mrobot: Fallo al registrar major\n");
        return major_number;
    }

    mrobot_class = class_create(CLASS_NAME);
    if (IS_ERR(mrobot_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(mrobot_class);
    }

    mrobot_device = device_create(mrobot_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mrobot_device)) {
        class_destroy(mrobot_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(mrobot_device);
    }

    cdev_init(&mrobot_cdev, &fops);
    mrobot_cdev.owner = THIS_MODULE;
    cdev_add(&mrobot_cdev, MKDEV(major_number, 0), 1);

    printk(KERN_INFO "mrobot: Driver cargado correctamente con major %d\n", major_number);
    return 0;
}

static void __exit mrobot_exit(void) {
    cdev_del(&mrobot_cdev);
    device_destroy(mrobot_class, MKDEV(major_number, 0));
    class_unregister(mrobot_class);
    class_destroy(mrobot_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "mrobot: Driver descargado\n");
}

module_init(mrobot_init);
module_exit(mrobot_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TuNombre");
MODULE_DESCRIPTION("Driver personalizado para mrobot");
MODULE_VERSION("1.0");
