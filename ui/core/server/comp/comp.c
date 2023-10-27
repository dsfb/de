
// comp.c
// The purpose of these routines is compose the final frame
// into the backbuffer and display it into the frontbuffer.

#include "../gwsint.h"

// The call back can't use the compose()
// if the display server is using it at the moment.
int __compose_lock = FALSE;

extern struct gws_window_d *mouse_hover;


// #todo
// Create some configuration globals here
// int gUseSomething = TRUE;
// ...

//old
static long __old_mouse_x=0;
static long __old_mouse_y=0;
//current
static long __new_mouse_x=0;
static long __new_mouse_y=0;
static int __mouse_initialized = FALSE;

static int refresh_pointer_status= FALSE;

// --------------------------

static void draw_mouse_pointer(void);

// --------------------------

// Refresh screen via kernel.
// Copy the backbuffer in the frontbuffer(lfb).
// #??
// It uses the embedded display server in the kernel.
//#define	SYSTEMCALL_REFRESHSCREEN        11
// #todo
// trocar o nome dessa systemcall.
// refresh screen será associado à refresh all windows.


// ??
// Using the kernel to show the backbuffer.
void gwssrv_show_backbuffer(void)
{
    gramado_system_call(11,0,0,0);
}

int flush_window(struct gws_window_d *window)
{
    //if ((void*) window == NULL)
        //return -1;
    return (int) gws_show_window_rect(window);
}

int flush_window_by_id(int wid)
{
    struct gws_window_d *w;
// wid
    if (wid<0 || wid >= WINDOW_COUNT_MAX){
        goto fail;
    }
// Structure validation
    w = (void*) windowList[wid];
    if ((void*) w == NULL){
        goto fail;
    }
// Flush
    flush_window(w);
    return 0;
    //return (int) flush_window(w);
fail:
    return (int) (-1);
}

// Flush the whole backbuffer.
// see: painter.c
void flush_frame(void)
{
    wm_flush_screen();
}

/*
 * reactRefreshDirtyWindows: 
 */
// Called by compose().
// O compositor deve ser chamado para compor um frame 
// logo após uma intervenção do painter, que reaje às
// ações do usuário.
// Ele não deve ser chamado X vezes por segundo.
// Quem deve ser chamado X vezes por segundo é a rotina 
// de refresh, que vai efetuar refresh dos retângulos sujos e
// dependendo da ação do compositor, o refresh pode ser da tela toda.
// Refresh
// Lookup the main window list.
// #todo: This is very slow. We need a linked list.
// Get next
// It is a valid window and
// it is a dirty window.
// Flush the window's rectangle and invalidate the window.
// see: rect.c

void reactRefreshDirtyWindows(void)
{
// Called by compose.
// + We need to encapsulate the variables used by this routine
//   to prevent about concorrent access problems.
// #bugbug
// This is not a effitient way of doing this.
// We got to refresh folowind the bottom top order.
// + If we refreshed the background window, so,we dont
//   need to refresh any other window when we're not using 
//   individual buffer for the windows in the compositor.

    register int i=0;
    // The component.
    // It's a window, but we don't care about its type.
    // All we need to do is refreshing the window's rectangle.
    struct gws_window_d *w;

// Is the root window a valid window

// Get the window pointer, refresh the windows retangle via KGWS and 
// validate the window.
    for (i=0; i<WINDOW_COUNT_MAX; ++i)
    {
        w = (struct gws_window_d *) windowList[i];
        if ((void*) w != NULL)
        {
            if ( w->used == TRUE && w->magic == 1234 )
            {
                if (w->dirty == TRUE)
                {
                    gws_refresh_rectangle ( 
                        w->absolute_x, 
                        w->absolute_y, 
                        w->width, 
                        w->height );

                    // We're done.
                    // We do the other windows in the next round.
                    if (w == __root_window){
                       validate_window(w);
                       return;
                    }
                    validate_window(w);
                }
            }
        }
    };
}

// wmReact:
// Refresh only the components that was changed by the painter.
// #todo
// Maybe in the future we can react to 
// changes in other components than windows.
void wmReactToPaintEvents(void)
{
// Refresh only the components that was changed by the painter.
// It means that we're reacting to all the paint events.

    reactRefreshDirtyWindows();
    // ...
}

/*
 * gws_show_window_rect:
 *     Mostra o retângulo de uma janela que está no backbuffer.
 *     Tem uma janela no backbuffer e desejamos enviar ela 
 * para o frontbuffer.
 *     A rotina de refresh rectangle tem que ter o vsync
 *     #todo: criar um define chamado refresh_window.
 */
// ??
// Devemos validar essa janela, para que ela 
// não seja redesenhada sem antes ter sido suja?
// E se validarmos alguma janela que não está pronta?
// #test: validando

int gws_show_window_rect(struct gws_window_d *window)
{
    //struct gws_window_d  *p;

    //#debug
    //debug_print("gws_show_window_rect:\n");

// Structure validation
    if ((void *) window == NULL){
        goto fail;
    }
    if (window->used != TRUE){
        goto fail;
    }
    if (window->magic != 1234){
        goto fail;
    }

//#shadow 
// ?? E se a janela tiver uma sombra, 
// então precisamos mostrar a sombra também. 
//#bugbug
//Extranhamente essa checagem atraza a pintura da janela.
//Ou talvez o novo tamanho favoreça o refresh rectangle,
//ja que tem rotinas diferentes para larguras diferentes

    //if ( window->shadowUsed == 1 )
    //{
        //window->width = window->width +4;
        //window->height = window->height +4;
        //refresh_rectangle ( window->left, window->top, 
        //    window->width +2, window->height +2 ); 
        //return (int) 0;
    //}

    //p = window->parent;


// Refresh rectangle
// See: rect.c   

    gws_refresh_rectangle ( 
        window->absolute_x, 
        window->absolute_y, 
        window->width, 
        window->height ); 

    validate_window(window);

    return 0;

fail:
    // #slow
    //debug_print("gws_show_window_rect: fail\n");
    return (int) -1;
}

void set_refresh_pointer_status(int value)
{
    if ( value != FALSE && 
         value != TRUE )
    {
        return;
    }
    refresh_pointer_status = value;
}

// Onde esta o mouse? em que janela?
// simple implementation.
// get the old one in the 32bit version of gramado.

void mouse_at(void)
{
    struct gws_window_d *w;
    register int i=0;
    for (i=0; i<WINDOW_COUNT_MAX; i++)
    {
        w = (void*) windowList[i];
        if ((void*) w != NULL)
        {
            if (w->magic == 1234)
            {
                if ( __new_mouse_x > w->absolute_x &&
                     __new_mouse_x < w->absolute_right &&
                     __new_mouse_y > w->absolute_y &&
                     __new_mouse_y > w->absolute_bottom )
                {
                    // Not the root
                    if (w != __root_window)
                    {
                        mouse_hover = (void *) w;
                        //redraw_window(w,TRUE);
                    }
                }
            }
        }
    };
}

long comp_get_mouse_x_position(void)
{
    return (long) __new_mouse_x;
}

long comp_get_mouse_y_position(void)
{
    return (long) __new_mouse_y;
}

// #todo
// We need to put this routine in another file.
// maybe mouse.c
static void draw_mouse_pointer(void)
{
    unsigned long rectLeft = __new_mouse_x;
    unsigned long rectTop = __new_mouse_y;
    unsigned long rectWidth = 8;
    unsigned long rectHeight = 8;
    unsigned int rectColor = COLOR_RED;
    unsigned long rectROP = 0;

    //int UseBMPImage= TRUE;

//
// BMP Image
//

    //if (UseBMPImage == TRUE)
    //{
        // #todo
        // Paint the paointer using a BMP Imange.
    //}

//
// Rectangle
//

// #todo: 
// print directly into the lfb.
// DRAW
    frontbuffer_draw_rectangle( 
        (unsigned long) rectLeft, (unsigned long) rectTop, 
        (unsigned long) rectWidth, (unsigned long) rectHeight, 
        (unsigned int) rectColor, 
        (unsigned long) rectROP );
}

// + Apaga o cursor antigo, copiando o conteudo do backbuffer
// + Pinta o novo cursor diretamente no lfb.
void __display_mouse_cursor(void)
{
    unsigned long rWidth = 16;
    unsigned long rHeight = 16;

    if ((void*) display_server == NULL)
        return;
    if (display_server->initialized != TRUE)
        return;

// #todo Limits
// Precisa inicializar os valores sobre o mouse.
// Precisa criar uma estrura pra eles.

    if ( __old_mouse_x<0 ){ __old_mouse_x=0; }
    if ( __old_mouse_y<0 ){ __old_mouse_y=0; }

    if ( __new_mouse_x<0 ){ __new_mouse_x=0; }
    if ( __new_mouse_y<0 ){ __new_mouse_y=0; }

//------
//#dangerdanger
//#todo: show the backbuffer

// Apaga se houve algum evento, como movimento.
    if (refresh_pointer_status == TRUE)
    {
        gws_refresh_rectangle( 
            __old_mouse_x, __old_mouse_y, rWidth, rHeight );
        set_refresh_pointer_status(FALSE);
    }

// save
    __old_mouse_x = __new_mouse_x;
    __old_mouse_y = __new_mouse_y;

// Draw direcly into the lfb.
    draw_mouse_pointer();
//------ 
}

// Flush
// The compositor.
// Called by wmCompose
void compose(void)
{
// Called by wmCompose() and callback_compose().

    static int Dirty = FALSE;

// fps++
    if (WindowManager.initialized == TRUE){
        WindowManager.frame_counter++;
    }

// Backgroud
// If the background is marked as dirty, 
// we flush it, validate it, show the cursor and return.
    Dirty = (int) is_background_dirty();
    if (Dirty == TRUE)
    {
        gws_show_backbuffer();
        validate_background();
        return;
    }

// Refresh only the components that was changed by the painter.
    wmReactToPaintEvents();

//
// #suspended
//

// mouse
// Show the cursor in the screen.
// remember: we're using 15 fps refresh,
// we can change this in the kernel.

    if (gUseMouse == TRUE){
        __display_mouse_cursor();
    }

// Validate the frame.
    validate_background();
    validate();
// fps
    //__update_fps();
}

// Called by the main routine for now.
// Its gonne be called by the timer.
// See: comp.c
void 
wmCompose(
    unsigned long jiffies, 
    unsigned long clocks_per_second )
{
    if (__compose_lock == TRUE)
        return;

    __compose_lock = TRUE;
    compose();
    __compose_lock = FALSE;
}

/*
// Marca como 'dirty' todas as janelas filhas,
// dai o compositor faz o trabalho de exibir na tela.
void refresh_subwidnows( struct gws_window_d *w );
void refresh_subwidnows( struct gws_window_d *w )
{
}
*/


// global
void comp_initialize_mouse(void)
{
    int hotspotx=0;
    int hotspoty=0;
    unsigned long w = gws_get_device_width();
    unsigned long h = gws_get_device_height();

    if (w>=0 && w<=800)
        hotspotx = (w >> 1);
    if (h>=0 && h<=800)
        hotspoty = (h >> 1);

    __old_mouse_x = hotspotx;
    __old_mouse_y = hotspoty;
    __new_mouse_x = hotspotx;
    __new_mouse_y = hotspoty;

    __mouse_initialized = TRUE; 

// CONFIG:
    //gUseMouse = TRUE;
    gUseMouse = FALSE;
}

// global
// Set new mouse position.
void comp_set_mouse_position(long x, long y)
{
    unsigned long w = gws_get_device_width();
    unsigned long h = gws_get_device_height();
    if ( x<0 ){ x=0; }
    if ( y<0 ){ y=0; }
    if ( x>w ){
         x=w;
    }
    if ( y>h ){
        y=h;
    }
    __new_mouse_x = (long) x;
    __new_mouse_y = (long) y;
}

