# Abusing access to mount namespaces through /proc/pid/root
原链接：https://labs.f-secure.com/blog/abusing-the-access-to-mount-namespaces-through-procpidroot/

> Containers are used to isolate workloads from the host system. In Linux, container runtimes such as Docker and LXC use multiple Linux namespaces to build an isolated environment for the workload.

容器用来把workloads和主机系统隔离开，在Linux中，Docker或者LXC这样的容器使用多个命名空间来构建一个用于workload的隔离的环境

> Due to this, a large part of the security research on containers and namespaces focus on exactly that: container breakouts. However, in some cases attackers are able to abuse containers and namespaces to escalate their privileges on an already compromised host. In this blog post we focus on such abuse.

一般研究都放在容器逃逸上，但是这篇文章主要介绍通过 滥用/攻击 容器来提权一个已经被入侵的主机。

> We show how Docker containers running with default privileges, without the '–privileged' flag can be abused for privilege escalation if an attacker has root in the container and a shell outside of the container. Additionally we describe how a common class of vulnerabilities, namely symlink following, can be made much worse with the use of namespaces.

这篇文章将展示，一个默认权限的Docker容器，如果攻击者有容器内的root权限，并且有一个容器外的shell的话，就可以进行提权。另外也将介绍，一个普通的漏洞（namely symlink following）如果通过命名空间利用，会变得更加危险。

## Quick intro to namespaces (namespaces 快速入门)

> The Linux kernel exposes seven namespaces that are used to isolate specific parts of the system from the workload. The PID namespace for example allows a process and its children to have an isolated view of the running processes on the system. The network namespace allows a set of processes to have their own view of the network, which is used to give containerized workloads their own IP address.

Linux内核公开了七个命名空间，用来将特定的部分与工作负载隔离开。比如`PID`命名空间和`network`命名空间，他们可以用来给容器的workloads自己的PID和IP address。

> Here we focus on the mount namespace and the user namespace. The former allows a set of processes to have their own view of the filesystem and the latter allows a user to gain access to actions previously only allowed for root users – as long as those actions only affect their own namespace.

这里重点关注`mount`命名空间和`user`命名空间。前者可以使得一些程序有自己的文件系统的视角，而后者可以在只影响自己的namespace的情况下，执行root user的操作。

> One special feature of the mount namespace is that they can be accessed through the /proc/PID/root/ and /proc/PID/cwd/ folders. These folders allow processes in a parent mount namespace and PID namespace to temporarily view files in the mount namespace of another process. This access is a bit magical and has some restrictions – for example, setuid executables will not work and device files are still usable even when /proc is mounted with the ‘nodev’ option.

`mount`命名空间有一个特性，它可以通过`/proc/PID/root/ and /proc/PID/cwd/` 文件夹来访问。这些文件夹允许父mount namespace和PID namespace 临时的访问另一个进程的`mount`命名空间的文件。这个操作有一定的限制，比如，如果`/proc`通过`nodev`挂在的话，setuid将不会生效并且设备文件仍然可以工作。 （？？？）

## Abusing Docker containers for privilege escalation (通过滥用Docker容器提权)

> Lets start by abusing the fact that nodev does not apply.

> In this scenario, we have an attacker with root within a Docker container and a shell on the host outside the Docker container. Docker containers do not use user namespaces; a root user within the container has root access outside the container. However, Docker removes a bunch of capabilities from the root user in a container to ensure that they cannot affect anything outside the container.

在nodev没有应用的情况下，我们具备之前的条件。Docker容器不会使用用户的命名空间；容器中的root用户在容器外具备root权限。但是Docker移除了root用户的一些能力来保证他们不能影响到container外部。

> By default, a Docker container has the following capabilities:
```
cap_chown,cap_dac_override,cap_fowner,cap_fsetid,cap_kill,cap_setgid,cap_setuid,cap_setpcap,
cap_net_bind_service,cap_net_raw,cap_sys_chroot,cap_mknod,cap_audit_write,cap_setfcap+eip
```

> Most of these capabilities are hard to abuse, for example, the cap_kill allows root in the container to kill all processes it can see, which is limited by the PID namespace, effectively only allowing processes within the container to be killed.

大多数的能力都很难被滥用，比如，cap_kill允许root用户kill掉所有它可以看到的进程，这些进程收到PID命名空间的限制。仅仅容器中的进程可以被kill掉。

> However, as the container has the cap_mknod, a root user within the container is allowed to create block device files. Device files are special files that are used to access underlying hardware & kernel modules. For example, the /dev/sda block device file gives access to read the raw data on the systems disk.

但是，`cap_mknod`可以用来创建一个块设备，用来访问底层硬件和内核模块。例如`/dev/sha`设备可以读系统磁盘上的raw data.

> Docker ensures that block devices cannot be abused from within the container by setting a cgroup policy on the container that blocks read and write of block devices.

Docker通过在容器上设置阻止读取和写入块设备的cgroup策略，确保不会从容器内部滥用块设备。

> However, if a block device is created within the container it can be accessed through the /proc/PID/root/ folder by someone outside the container, the limitation being that the process must be owned by the same user outside and inside the container.

但是，如果一个容器内的块设备可以通过`/proc/PID/root`被容器外的某人访问，这个限制就会变成这个进程必须被同样的用户拥有（在容器内和容器外）。

> An example of the attack is demonstrated in the screenshot below. On the left is the attacker with root within the container and on the right is the attacker without root on the host.

<img src = "https://labs.f-secure.com/assets/Uploads/_resampled/ResizedImageWzg2MSw0NDJd/pic12.png"></img>

> This attack is easily prevented by following best practices by ensuring that nobody is root within the container and by running Docker with the parameter '–cap-drop=MKNOD'.

这个攻击可以通过不给root权限或者是取消`cap_mknod`来防止。

整理一下，这个利用的条件是： M<br />
1. `nodev`没有应用
2. 有容器内的root权限
3. 有容器外的一个shell

利用的结果是： <br />
完全控制filesystem

## Enhancing symlink vulnerabilities


参考资料：
[mount_namespace](https://www.cnblogs.com/sparkdev/p/9424649.html)


# Helping root out of the container

> In the previous post we described how to perform privilege escalation using an unprivileged shell outside a container and a root shell within an unprivileged docker container. The method used in that post relied on the ability to create device files within the container and pass the device file to the user outside the container through /proc/PID/root.

前面介绍的方法需要在容器内部创建一个设备，然后通过`/proc/PID/root`将这个设备传递给外面的用户。

> In this post we show another method that, instead of smuggling a device file out of the container - we smuggle a file descriptor into the container. Once root has access to the file descriptor they are able to perform actions on the referenced file even if it is outside the mount namespace of the containerized process.

这里介绍另一个方法。传一个文件描述符进去，一旦root可以访问这个文件描述符，那么就可以对它进行操作，尽管这个文件是在容器的外部。

> Both of these attacks are relevant in a setting where someone is trying to provide an unprivileged user with a limited root by using containers. One example of this would be in a managed laptop used by a developer. Developers usually want root on their machine while the local IT-organization doesn't want to give root to employees. In such a scenario it may be tempting to give the developer access to root within a container, which we again show is not a good idea.

这些攻击都和为非特权用户提供容器限制的root权限有关。一个例子是开发者希望得到root权限但是组织不希望给他们。这种情况下可能会给开发者一个容器的root权限。我们再次证明了，这不是一个好主意。

## The breakout

> The base of this method uses the AF_LOCAL sockets, also known as unix domain sockets. These are special sockets that are placed on the filesystem instead of being bound to specific ports on a network device or IP address. Clients that want to access the service listening on a socket points to said file when creating their socket.

这个方法的基础是使用`AF_LOCAL`套接字，也被称为是`unix domain socket`，这些套接字十分特别，因为他们不是被绑定在一个网络设备的端口或者ip地址上，而是被放在文件系统中。想要访问侦听套接字的服务的客户端在创建其套接字时会指向该文件。

> Now a nice part of such sockets is that one can send file descriptors over them. If you read the last post, we discussed how root within a container is limited because they can't address files outside the container because of the use of a mount namespace. In this post we describe how to pass a file descriptor into the container, allowing root in the container to actually address files within.

这类套接字可以用来传递文件描述符。

> Let's get to it! To abuse the AF_LOCAL socket to help root out of a container we do the following:
1. In container, as root: Create AF_LOCAL socket listening on file /socket
2. In container, as UID of user outside container: Launch process, save pid Step2_PID
3. Outside container: Connect to the /socket file through /proc/Step2_PID/root/socket
4. Outside container: Send file descriptor for root directory /
5. In container, as root: Receive file descriptor and modify files outside container

> And to make this process simple an example program is available at: https://github.com/FSecureLABS/fdpasser. The example program does a chmod 6777 on a file outside of the container, in the example below this file is /etc/shadow.