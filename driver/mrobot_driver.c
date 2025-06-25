#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "mrobot"
#define CLASS_NAME  "mrobot_class"

static int major;
static struct class*  mrobot_class  = NULL;
static struct device* mrobot_device = NULL;
static struct cdev mrobot_cdev;

#define BUF_SIZE 64
static char command_buf[BUF_SIZE];

// Prototipos
static int     mrobot_open(struct inode *, struct file *);
static int     mrobot_release(struct inode *, struct file *);
static ssize_t mrobot_write(struct file *, const char __user *, size_t, loff_t *);

// Tabla de operaciones del driver
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mrobot_open,
    .release = mrobot_release,
    .write = mrobot_write,
};

// Abrir el dispositivo
static int mrobot_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "mrobot: Dispositivo abierto\n");
    return 0;
}

// Cerrar el dispositivo
static int mrobot_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "mrobot: Dispositivo cerrado\n");
    return 0;
}

// Escribir en el dispositivo (recibe comandos desde user space)
static ssize_t mrobot_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
    if (len > BUF_SIZE - 1)
        len = BUF_SIZE - 1;
    if (copy_from_user(command_buf, buf, len))
        return -EFAULT;
    command_buf[len] = '\0';

    // Aquí parseas el comando recibido, por ejemplo: "123,456,1,P\n"
    // y controlas los motores y el servo usando GPIO, I2C, SPI, etc.
    // Ejemplo: printk(KERN_INFO "mrobot: Recibido comando: %s\n", command_buf);

    // TODO: Implementa aquí el control físico de los motores y el servo
    // usando las APIs del kernel para GPIO, PWM, etc.

    printk(KERN_INFO "mrobot: Recibido comando: %s\n", command_buf);

    return len;
}

// Inicialización del módulo
static int __init mrobot_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "mrobot: Fallo al registrar el major number\n");
        return major;
    }
    mrobot_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mrobot_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "mrobot: Fallo al crear la clase\n");
        return PTR_ERR(mrobot_class);
    }
    mrobot_device = device_create(mrobot_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mrobot_device)) {
        class_destroy(mrobot_class);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "mrobot: Fallo al crear el dispositivo\n");
        return PTR_ERR(mrobot_device);
    }
    cdev_init(&mrobot_cdev, &fops);
    cdev_add(&mrobot_cdev, MKDEV(major, 0), 1);
    printk(KERN_INFO "mrobot: Driver inicializado correctamente\n");
    return 0;
}

// Salida del módulo
static void __exit mrobot_exit(void) {
    cdev_del(&mrobot_cdev);
    device_destroy(mrobot_class, MKDEV(major, 0));
    class_unregister(mrobot_class);
    class_destroy(mrobot_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "mrobot: Driver eliminado\n");
}

module_init(mrobot_init);
module_exit(mrobot_exit);

MODULE_LICENSE("GPL");   
MODULE_AUTHOR("Valerin y Yendry");
MODULE_DESCRIPTION("Driver de caracter para brazo robotico mrobot");