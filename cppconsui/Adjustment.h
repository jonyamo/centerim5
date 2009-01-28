/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif*/

#ifndef __ADJUSTMENT_H__
#define __ADJUSTMENT_H__


#include <gdk/gdk.h>
#include <gtk/gtkobject.h>

//G_BEGIN_DECLS

/*
#define GTK_TYPE_ADJUSTMENT                  (gtk_adjustment_get_type ())
#define GTK_ADJUSTMENT(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_ADJUSTMENT, GtkAdjustment))
#define GTK_ADJUSTMENT_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_ADJUSTMENT, GtkAdjustmentClass))
#define GTK_IS_ADJUSTMENT(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_ADJUSTMENT))
#define GTK_IS_ADJUSTMENT_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ADJUSTMENT))
#define GTK_ADJUSTMENT_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_ADJUSTMENT, GtkAdjustmentClass))
*/

//typedef struct _GtkAdjustment	    GtkAdjustment;
//typedef struct _GtkAdjustmentClass  GtkAdjustmentClass;

class Adjustment
{
	public:
		Adjustment (	gdouble	  value,
				gdouble	  lower,
				gdouble	  upper,
				gdouble	  step_increment,
				gdouble	  page_increment,
				gdouble	  page_size);



void	   changed		(GtkAdjustment	 *adjustment);
void	   value_changed		(void);
void	   clamp_page		(
						 gdouble	  lower,
						 gdouble	  upper);

gdouble	   get_value		(void);
void	   set_value		(
						 gdouble	  value);
gdouble    get_lower             (void);
void       set_lower             (GtkAdjustment   *adjustment,
                                                 gdouble          lower);
gdouble    get_upper             (void);
void       set_upper             (
                                                 gdouble          upper);
gdouble    get_step_increment    (void);
void       set_step_increment    (gdouble          step_increment);
gdouble    get_page_increment    (void);
void       set_page_increment    ( gdouble          page_increment);
gdouble    get_page_size         (void);
void       set_page_size         ( gdouble          page_size);

void       configure           ( gdouble          value,
				 gdouble          lower,
				 gdouble          upper,
				 gdouble          step_increment,
				 gdouble          page_increment,
				 gdouble          page_size);


	protected:

	private:
		Adjustment (void);

	gdouble lower;
	gdouble upper;
	gdouble value;
	gdouble step_increment;
	gdouble page_increment;
	gdouble page_size;

  void changed	 (void);
  void value_changed (void);


};

#endif /* __ADJUSTMENT_H__ */
