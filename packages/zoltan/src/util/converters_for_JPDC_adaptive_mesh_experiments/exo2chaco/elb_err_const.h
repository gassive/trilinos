/*====================================================================
 * ------------------------
 * | CVS File Information |
 * ------------------------
 *
 * $RCSfile$
 *
 * $Author$
 *
 * $Date$
 *
 * $Revision$
 *
 * $Name$
 *====================================================================*/

#ifndef _EXOIILB_ERR_CONST_H_
#define _EXOIILB_ERR_CONST_H_

#ifndef lint
static char *cvs_errch_id = "$Id$";
#endif

#define MAX_ERR_MSG 1024

/* Structure to store an error message */
struct error_message
{
  int   level;
  char *err_mesg;
  int   error_level;
  int   line_no;
  char *filename;
};

typedef struct error_message ERROR_MSG;
typedef struct error_message *ERROR_MSG_PTR;

extern int error_lev;

/* Macro used in the code to add an error message */
#define Gen_Error(a,b) (error_add(a, b, __FILE__, __LINE__))

/* Function prototype for error functions */
extern
void error_add(
  int   level,
  char *message,	/* The message to add to the error list */
  char *filename,	/* The filename in which the error occured */
  int   line		/* The line number in filename where the error
                         * was reported */
  );

extern
void error_report(void);

#endif /* _EXOIILB_ERR_CONST_H_ */
