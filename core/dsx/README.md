# DSX - Display server.

```
 * DSX Display Server.
 * DSX.BIN
 * codename: dsx
 * Created by Fred Nora.
```

  The goal for this extra display server is testing the client-side library libx, 
where we're use some of the functions found in the X protocol. Just like XNextEvent().

```
  DSX is a 'Display Server'. A display server provides a way for the applications to draw 
  in the display device, and receive input events from keyboard and mouse devices. 
  The display server needs to access the display device driver to touch 
  the real display device.

  In other words, the display server is a bridge between the 'application' and 
  the 'display device driver'.

  |Display device driver| <---- |Display server| <---- |Application|

  The client-side applications are gonna call the display server to create windows and 
  draw strings in to the screen.
```

```
  pt-br:
  O proposito de um display server e' prestar serviços para os cliente 
  atraves dos serviços oferecidos pelo driver de display device, pelo driver 
  de dispositivo teclado e pelo driver de dispositivo mouse.
  Basicamente oferece acessoa tela, ao teclado e ao mouse.
  Entao quer dizer que esse tipo de servidor nao e' propriamente um 
  servidor de janelas ... pois dar acesso a janelas e' apenas parte do serviço.
  Sendo assim, daqui pra frente meu servidor de janelas sera tratado 
  como um display server. Me parece que esse e' o termo mais usado. 
  Ou seja, Um 'desktop environment' como o gnome, utiliza um 'window system' 
  e esse window system possui um display server. 
  No Wayland o display server e' o compositor e 
  no Xorg o display server e' o X server.
```









