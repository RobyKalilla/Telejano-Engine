/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cvar.c -- dynamic variable tracking

#include "quakedef.h"


cvar_t *Cvar_Set2 (char *var_name, char *value, qboolean force);//Q2 set support

cvar_t	*cvar_vars;
char	*cvar_null_string = "";

// Q2!
#define	CVAR_ARCHIVE	1	// set to cause it to be saved to vars.rc
#define	CVAR_USERINFO	2	// added to userinfo  when changed
#define	CVAR_SERVERINFO	4	// added to serverinfo when changed
#define	CVAR_NOSET		8	// don't allow change from console at all,
							// but can be set from the command line
#define	CVAR_LATCH		16	// save changes until server restart

#define CVAR_SERVER     4
#define CVAR_USERDEF    16  // Data archive as new


// Q2!



/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (char *var_name)
{
	cvar_t	*var;
	
	for (var=cvar_vars ; var ; var=var->next)
		if (!Q_strcmp (var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float	Cvar_VariableValue (char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return Q_atof (var->string);
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}


/*
============
Cvar_CompleteVariable
============
*/
char *Cvar_CompleteVariable (char *partial)
{
	cvar_t		*cvar;
	int			len;
	
	len = Q_strlen(partial);
	
	if (!len)
		return NULL;
		
// check functions
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!Q_strncmp (partial,cvar->name, len))
			return cvar->name;

	return NULL;
}

// Tomaz - Enhanced Complete Command Begin
/*
============
Cvar_CompleteCountPossible
============
*/
int Cvar_CompleteCountPossible (char *partial)
{
	cvar_t	*cvar;
	int	len;
	int	h;

	h=0;

	len = Q_strlen(partial);
	
	if (!len)
		return 0;
		
	// Loop through the cvars and count all partial matches
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!Q_strncmp (partial,cvar->name, len))
			h++;
	return h;
}

/*
============
Cvar_CompletePrintPossible
============
*/
void Cvar_CompletePrintPossible (char *partial)
{
	cvar_t	*cvar;
	int	len;
	int	lpos;
	int	out;
	int	con_linewidth;
	char	sout[32];
	char	lout[2048];

	len = Q_strlen(partial);
	lpos = 0;
	Q_strcpy(lout,"");

	// Determine the width of the console
	con_linewidth = (vid.width >> 3) - 3;

	// Loop through the cvars and print all matches
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!Q_strncmp (partial,cvar->name, len))
		{
			Q_strcpy(sout, cvar->name);

			out = Q_strlen(sout);
			lpos += out;

			// Pad with spaces
			for (out; out<20; out++)		
			{
				if (lpos < con_linewidth)
					Q_strcat (sout, " ");
				
				lpos++;
			}

			Q_strcat (lout, sout);

			if (lpos > con_linewidth - 24)
				for  (lpos; lpos < con_linewidth; lpos++)
					Q_strcat(lout, " ");

			if (lpos >= con_linewidth)
				lpos = 0;
		}
	Con_Printf ("%s\n\n", lout);
}
// Tomaz - Enhanced Complete Command End

/*
============
Cvar_Set
============
*/
void Cvar_Set (char *var_name, char *value)
{
	cvar_t	*var;
	qboolean changed;

	
	var = Cvar_FindVar (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	changed = Q_strcmp(var->string, value);

#if 1 //Tei speedup from Lh
	if(!changed)
		return;

	// LordHavoc: don't reallocate when the buffer is the same size
	if (!var->string || strlen(var->string) != strlen(value))
	{
		Z_Free (var->string);	// free the old value string

		var->string = Z_Malloc (strlen(value)+1);
	}
#else
	Z_Free (var->string);	// free the old value string

	
	var->string = Z_Malloc (Q_strlen(value)+1);
#endif //Tei speedupt cvars	

#if 1 //without Linked Cvars
	Q_strcpy (var->string, value);
	var->value = Q_atof (var->string);

	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}
	if ((var->value != 0) && (var == &deathmatch))
		Cvar_Set ("coop", "0");

	if ((var->value != 0) && (var == &coop))
		Cvar_Set ("deathmatch", "0");
#else //Tei linked cvars
	Q_strcpy (var->string, value);
	var->value = Q_atof (var->string);

	if (var->linked && var->linked->name)
	{

		if( var->linked->linked && var->linked->linked == &var)
		{
			var->linked->norecursive = true;	
		}

		if( var->norecursive)
			var->norecursive = false;
		else
			Cvar_Set(var->linked->name,value);//the think, recursive
	}

	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}

#endif
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (char *var_name, float value)
{
	char	val[32];
	
	sprintf (val, "%f",value);
	Cvar_Set (var_name, val);
}


/*
============
Cvar_RegisterVariable

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterVariable (cvar_t *variable)
{
	char	*oldstr;
	
// first check to see if it has allready been defined
	if (Cvar_FindVar (variable->name))
	{
		Con_Printf ("Can't register variable %s, allready defined\n", variable->name);
		return;
	}
	
// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name);
		return;
	}
		
// copy the value off, because future sets will Z_Free it
	oldstr = variable->string;
	variable->string = Z_Malloc (Q_strlen(variable->string)+1);	
	Q_strcpy (variable->string, oldstr);
	variable->value = Q_atof (variable->string);
	
// link the variable in
	variable->next = cvar_vars;
	cvar_vars = variable;
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean	Cvar_Command (void)
{
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;
		
// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set (v->name, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;
	
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->archive)
				fprintf (f, "%s \"%s\"\n", var->name, var->string);
		if (var->userdef )
				fprintf (f, "set %s \"%s\"\n", var->name, var->string);
	}

}

// Q2! cvarlist

/*
============
Cvar_List_f

============
*/
void Cvar_List_f (void)
{
	cvar_t	*var;
	int		i;

	i = 0;
	for (var = cvar_vars ; var ; var = var->next, i++)
	{
		if (var->archive)
			Con_Printf ("*");
		else
			Con_Printf (" ");
				
		if (var->server)
			Con_Printf ("S");
		else
			Con_Printf (" ");

		if (var->userdef)
			Con_Printf ("U");
		else
			Con_Printf (" ");

		Con_Printf (" %s \"%s\"\n", var->name, var->string);
	}
	Con_Printf ("%i cvars\n", i);
}

// Q2!  cvarlist

// Q2!


char *CopyString (char *in);

/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
void Cvar_Flags ( cvar_t * var, int flags )
{
	if (flags & CVAR_SERVER )
		var->server = true;

	if (flags & CVAR_ARCHIVE )
		var->archive = true;

	if (flags & CVAR_USERDEF )
		var->userdef = true;

	var->linked = NULL;//Tei linked cvars
	var->norecursive = false; //Tei linked cvars
}

static qboolean Cvar_InfoValidate (char *s)
{
	if (strstr (s, "\\"))
		return false;
	if (strstr (s, "\""))
		return false;
	if (strstr (s, ";"))
		return false;
	return true;
}


cvar_t *Cvar_Get (char *var_name, char *var_value, int flags)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (var)
	{
		if (flags & CVAR_USERDEF)
			flags -=  CVAR_USERDEF;
		Cvar_Flags(var,flags);
		return var;
	}
	
	if (!var_value)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_value))
		{
			Con_Printf("invalid info cvar value\n");
			return NULL;
		}
	}

	var = Z_Malloc (sizeof(*var));
	var->name = CopyString (var_name);
	var->string = CopyString (var_value);
	//var->modified = true;
	var->value = atof (var->string);

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	Cvar_Flags(var,flags);

	return var;
}




cvar_t *Cvar_FullSet (char *var_name, char *value, int flags)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		return Cvar_Get (var_name, value, flags);
	}

	//var->modified = true;
	Cvar_Flags(var,flags);
	/*
	if (flags & CVAR_SERVER )
		var->server = true;

	if (flags & CVAR_ARCHIVE )
		var->archive = true;
	*/
	Z_Free (var->string);	// free the old value string
	
	var->string = CopyString(value);
	var->value = atof (var->string);

	return var;
}

/*
============
Cvar_Set2
============
*/
cvar_t *Cvar_Set2 (char *var_name, char *value, qboolean force)
{
	cvar_t	*var;

	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		var = Cvar_Get (var_name, value, CVAR_USERDEF);
		var->userdef = true;
		return var;
	}

	if (var->server | var->archive)
	{
		if (!Cvar_InfoValidate (value))
		{
			Con_Printf("invalid info cvar value\n");
			return var;
		}
	}

	if (!strcmp(value, var->string))
		return var;		// not changed
	
	Z_Free (var->string);	// free the old value string
	
	var->string = CopyString(value);
	var->value = atof (var->string);

	return var;
}



void Cvar_Set_f (void)
{
	int		c;
	int		flags;

	c = Cmd_Argc();
	if (c != 3 && c != 4)
	{
		Con_Printf ("usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4)
	{
		if (!strcmp(Cmd_Argv(3), "a"))
			flags = CVAR_ARCHIVE;
		else if (!strcmp(Cmd_Argv(3), "s"))
			flags = CVAR_SERVER;
		else
		{
			Con_Printf ("flags can only be 'a' or 's'\n");
			return;
		}
		Cvar_FullSet (Cmd_Argv(1), Cmd_Argv(2), flags);
	}
	else
		Cvar_Set2 (Cmd_Argv(1), Cmd_Argv(2), true);
}



void Cvar_Inc_f(void) {
	cvar_t	*var;
	char * cvarname;
	char newval[1000];
	int		c, val;
	

	c = Cmd_Argc();
	cvarname =Cmd_Argv(1);

	if (c != 2 )
	{
		Con_Printf ("usage: inc <variable>\n");
		return;
	}

	var = Cvar_FindVar (cvarname);

	if (!var) 
	{
		Con_Printf ("'%s' not found\n", cvarname);
		return;
	}

	val = var->value + 1;

	sprintf(newval,"%d",val);

	Cvar_FullSet ( cvarname, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 


void Cvar_Dec_f(void) {
	cvar_t	*var;
	char * cvarname;
	char newval[1000];
	int		c, val;
	

	c = Cmd_Argc();
	cvarname =Cmd_Argv(1);

	if (c != 2 )
	{
		Con_Printf ("usage: dec <variable>\n");
		return;
	}

	var = Cvar_FindVar (cvarname);

	if (!var) 
	{
		Con_Printf ("'%s' not found\n", cvarname);
		return;
	}

	if (var->value)
		val = var->value - 1;

	sprintf(newval,"%d",val);

	Cvar_FullSet ( cvarname, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 



void Cvar_Mov_f(void) {
	cvar_t	*var1;
	cvar_t	*var2;

	char * cvardst;
	char * cvarsrc;

	char newval[1000];
	int		c, val;
	

	c = Cmd_Argc();
	cvardst =Cmd_Argv(1);
	cvarsrc =Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: mov <dst> <src>\n");
		return;
	}

	var1 = Cvar_FindVar (cvardst);
	var2 = Cvar_FindVar (cvarsrc);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}

	if (!var2) 
	{
		Con_Printf ("'%s' or not found\n", cvarsrc);
		return;
	}


	val = var2->value;

	sprintf(newval,"%d",val);

	Cvar_FullSet ( cvardst, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 


void Cvar_AddVars_f(void) {
	cvar_t	*var1;
	cvar_t	*var2;

	char * cvardst;
	char * cvarsrc;

	char newval[1000];
	int		c;
	float val;
	

	c = Cmd_Argc();
	cvardst =Cmd_Argv(1);
	cvarsrc =Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: add <dst> <src>\n");
		return;
	}

	var1 = Cvar_FindVar (cvardst);
	var2 = Cvar_FindVar (cvarsrc);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}

	if (!var2) // var2 is not a cvar
	{
		sscanf(cvarsrc,"%f",&val); //assume is a constant value
		val = var1->value + val;
	}
	else
		val = var2->value + var1->value;

	sprintf(newval,"%f",val);

	Cvar_FullSet ( cvardst, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 


void Cvar_MultVars_f(void) {
	cvar_t	*var1;
	cvar_t	*var2;

	char * cvardst;
	char * cvarsrc;

	char newval[1000];
	int		c;
	float val;
	

	c = Cmd_Argc();
	cvardst =Cmd_Argv(1);
	cvarsrc =Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: mul <dst> <src>\n");
		Con_Printf ("       mul <dst> <value>\n");
		Con_Printf (" will be <dst> = <dst> * <src> \n");
		return;
	}

	var1 = Cvar_FindVar (cvardst);
	var2 = Cvar_FindVar (cvarsrc);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}

	if (!var2) 
	{
		sscanf(cvarsrc,"%f",&val);
		val = var1->value * val;
	}
	else 
		val = var2->value * var1->value;

	sprintf(newval,"%f",val);

	Cvar_FullSet ( cvardst, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 



void Cvar_BoundVar_f(void) {
	cvar_t	*var1;

	char * cvar;
	char *min, *max;

	char newval[1000];
	int		c;
	float val, fmin, fmax;
	

	c		= Cmd_Argc();
	cvar	= Cmd_Argv(1);
	min		= Cmd_Argv(2);
	max		= Cmd_Argv(3);

	if (c < 2 )
	{
		Con_Printf ("usage: bound <cvar> <min> <max>\n");
		Con_Printf ("       bound <cvar> <min>\n");
		Con_Printf ("       bound <cvar>\n");
		Con_Printf (" Default <max> is 8192, default <min> is 0\n");
		return;
	}


	var1 = Cvar_FindVar (cvar);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvar);
		return;
	}

	if (c<4)
	{	
		fmax = 8192;	
	}
	else
		sscanf(max  ,"%f",&fmax  );

	if (c<3)
	{	
		fmin = 0;	
	}
	else
		sscanf(min  ,"%f",&fmin  );

	val = var1->value ;

	if (val<fmin)
		val = fmin;
	else
	if (val>fmax)
		val = fmax;

	sprintf(newval,"%f",val);

	Cvar_FullSet ( cvar, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 


char* MakeSlider(float max, float dx);

void Cvar_Slider_f(void) {

	cvar_t * from;
	char * cvardst;
	char * value;
	char * max;

	int		c;
	float fmax, fvalue;
	

	c		= Cmd_Argc( );
	cvardst = Cmd_Argv(1);
	value	= Cmd_Argv(2);
	max		= Cmd_Argv(3);

	if (c < 2 )
	{
		Con_Printf ("usage: slider <cvar dst> <cvar value> <max>\n");
		Con_Printf ("       slider <cvar dst> <cvar value>\n");
		Con_Printf ("       slider <cvar dst>\n");
		Con_Printf (" max default is 10, <cvar value> default is <cvar dst>\n");
		return;
	}

	if (c >2)
	{
		from = Cvar_FindVar (value);

		if (!from) 
		{
			Con_Printf ("'%s' or not found\n", value);
			return;
		}
		fvalue = from->value;
	}
	else
	{
		from = Cvar_FindVar (cvardst);

		if (!from) 
		{
			Con_Printf ("'%s' or not found\n", cvardst);
			return;
		}
		fvalue = from->value;
	}

	if (c> 3)
	{
		sscanf(max  ,"%f",&fmax  );
	}
	else
	   fmax = 10;	


	Cvar_FullSet ( cvardst, MakeSlider(fmax,fvalue),0);
} 

static char	buf[2000];//for concat bussines

void Cvar_ComboBox_f(void) {

	cvar_t * dest, * from, *index;
	char * cvardst;
	char * value;
	char * max;

	int		c,m,t,lenvalue;
	float findex=0;
	

	c		= Cmd_Argc( );
	cvardst = Cmd_Argv(1);
	value	= Cmd_Argv(2);
	max		= Cmd_Argv(3);

	if (c != 4 )
	{
		Con_Printf ("usage: combobox <cvar dst> <cvar string> <cvar index>\n");
		return;
	}

	dest = Cvar_FindVar (cvardst);

	if (!dest) 
	{
			Con_Printf ("'%s' or not found\n", cvardst);
			return;
	}

	from = Cvar_FindVar (value);

	if (from) 
			value = from->string;	

	index = Cvar_FindVar (max);

	if (index) 
			max = index->string;	

	//read index 
	sscanf(max,"%f",&findex);

	//Con_Printf(" index is %f, str is %s\n",findex, value);
	//select combo item
	
	lenvalue = strlen( value );
	m = 0;
	c = 0;
	for (t=0;t<lenvalue;t++)
	{
		if (value[t]=='!')
			m ++;
		if (findex == m && value[t]!='!')
		{
			buf[c++] = value[t];
			if (value[t] =='_')
				buf[c-1] =' ';
		}
	}
	buf[c]=0;

	//clone buf to cvar
	Cvar_FullSet ( cvardst, buf,0);
} 


void Cvar_ConCat_f(void) {

	cvar_t * from, * to;
	char * cvardst;
	char * value;

	int		c;
	

	c		= Cmd_Argc( );
	cvardst = Cmd_Argv(1);
	value	= Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: concat <cvar dst> <cvar value>\n");
		Con_Printf ("       concat <cvar dst> <string literal>\n");
		return;
	}

	from = Cvar_FindVar (value);
	to	 = Cvar_FindVar (cvardst);

	if (!to)
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}

	if (!from) 
		sprintf(buf,"%s%s",to->string,value);
	else	
		sprintf(buf,"%s%s",to->string,from->string);

	Cvar_FullSet ( cvardst,buf ,0);
} 


void Cvar_Link_f(void) {

	cvar_t * from, * to;
	char * cvardst;
	char * value;

	int		c;
	

	c		= Cmd_Argc( );

	cvardst = Cmd_Argv(1);to   = Cvar_FindVar (cvardst);
	value	= Cmd_Argv(2);from = Cvar_FindVar (value);		

	if( c!= 2 && c!=3)
	{
		Con_Printf ("usage: link <cvar org> <cvar reflex>\n");
		Con_Printf ("       link <cvar to actualize>\n");
	}

	if (!to)
	{
			Con_Printf ("'%s' or not found\n", cvardst);
			return;
	}

	switch(c) 
	{
	case 3:
		if (!from)
		{
			Con_Printf ("'%s' or not found\n",value );
			return;
		}

		to->linked = from;//linking 
		break;
	case 2:
		if (to->linked && to->linked->name)
		{
				Cvar_Set(to->linked->name,to->string);//the think, recursive
		}
		break;
	default:
		break;
	}
} 





void Cvar_Accum_f(void) {
	cvar_t	*var1;
	//cvar_t	*var2;

	int ival;

	char * cvardst;
	char * cvarsrc;

	char newval[1000];
	int		c, val;
	

	c = Cmd_Argc();
	cvardst =Cmd_Argv(1);
	cvarsrc =Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: accum <dst> <bit>\n");
		return;
	}

	var1 = Cvar_FindVar (cvardst);
	//var2 = Cvar_FindVar (cvarsrc);

	 sscanf(cvarsrc,"%d",&ival);

	//if (!var1) 
	//{
	//	Con_Printf ("'%s' or not found\n", cvardst);
	//	return;
	//}

	//Con_Printf("bit is %d for %s\n", ival, cvarsrc);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}


	val = (int)(var1->value) | (1<<(ival));

	sprintf(newval,"%d",val);

	Cvar_FullSet ( cvardst, newval,0);
	Cvar_FullSet ( "temp2", newval,0);
} 


void Cvar_Reset_f(void) {
	cvar_t	*var1;
	//cvar_t	*var2;

	int ival, ibit, i;

	char * cvardst;
	char * cvarsrc;

	char newval[1000];
	int		c;//, val;
	

	c = Cmd_Argc();
	cvardst =Cmd_Argv(1);
	cvarsrc =Cmd_Argv(2);

	if (c != 3 )
	{
		Con_Printf ("usage: reset <dst> <bit>\n");
		return;
	}

	var1 = Cvar_FindVar (cvardst);
	//var2 = Cvar_FindVar (cvarsrc);

	 sscanf(cvarsrc,"%d",&ival);

	//if (!var1) 
	//{
	//	Con_Printf ("'%s' or not found\n", cvardst);
	//	return;
	//}

	//Con_Printf("bit is %d for %s\n", ival, cvarsrc);

	if (!var1) 
	{
		Con_Printf ("'%s' or not found\n", cvardst);
		return;
	}

	i = (var1->value);
    ibit = (1<<ival);


	if ( i & ibit )
		i = i - ibit;

	sprintf(newval,"%d",i);

	Cvar_FullSet ( cvardst, newval,0);
	Cvar_FullSet ( "temp2", newval,0);

} 

extern int eval_trigger;

void Cvar_Trigger_f(void) {
	//cvar_t	*var1;

	//int ival;//, ibit;
	eval_t	*val;
	int			i;//, flags;
	edict_t		*e;
	
	char * trig;

	//char newval[1000];
	int		c;
	

	c = Cmd_Argc();
	trig =Cmd_Argv(1);

	if (c != 2 )
	{
		Con_Printf ("usage: trigger <trig> \n");
		return;
	}

	if (!eval_trigger)
		return;

	for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		val = GETEDICTFIELDVALUE(e, eval_trigger);
		if (val->string)
		{
			if (!strcmp(trig,(val->string + pr_strings)) )
			{
				//pr_global_struct->self = EDICT_TO_PROG();
				//pr_global_struct->other = EDICT_TO_PROG(e);
				pr_global_struct->self = EDICT_TO_PROG(sv_player);
				if (e->v.touch)
				{
					PR_ExecuteProgram (e->v.touch);
					Con_Printf("%d:%s Touch!\n",i,(val->string + pr_strings));
				}
				else
					Con_Printf("%d:%s\n",i,(val->string + pr_strings));

				/*if( e->v.use)
				{
					Con_Printf("%d:%s use!\n",i,(val->string + pr_strings));
					PR_ExecuteProgram (e->v.use);
				}*/

			}
		}
	}
} 


void Cvar_TriggerList_f(void) {
	//cvar_t	*var1;
	//cvar_t	*var2;

	//int ival, ibit;
	eval_t	*val;
	int			i;//, flags;
	edict_t		*e;

	
	if (!eval_trigger)
		return;


	for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		val = GETEDICTFIELDVALUE(e, eval_trigger);
		if (val->string)
		{
			Con_Printf("%d:%s\n",i,(val->string + pr_strings));

		}
	}
} 


/*
============
Cvar_Toggle_f -- johnfitz
============
*/
void Cvar_Toggle_f (void)
{
	float actual, max;
	cvar_t * cvmax;
	char val[1000];//Temporal buffer

	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("toggle <cvar> : toggle cvar\n");
		break;
	case 2:
		if (Cvar_VariableValue(Cmd_Argv(1)))
			Cvar_Set (Cmd_Argv(1), "0");
		else
			Cvar_Set (Cmd_Argv(1), "1");
		break;
	case 3:
		
		actual =  Cvar_VariableValue(Cmd_Argv(1));
		cvmax = Cvar_FindVar (Cmd_Argv(2));

		if (!cvmax)
			sscanf(Cmd_Argv(2),"%f",&max);
		else
			max = cvmax->value;

		actual++;

		if (actual>max)
			actual = 0;
		sprintf(val,"%f", actual);

		Cvar_Set (Cmd_Argv(1), val);	
		break;
	}
}


void Cvar_ToggleText_f (void)
{
	int len, i;
	cvar_t * from;
	char * value;
	char max[1000];

	int		c;
	

	c		= Cmd_Argc( );
	value	= Cmd_Argv(1);

	if (c != 2 )
	{
		Con_Printf ("usage: toggletext <cvar>\n");
		return;
	}

	from = Cvar_FindVar (value);

	if (!from) 
	{
		Con_Printf ("'%s' or not found\n", value);
		return;
	}

	strcpy(max,from->string);
	len = strlen(max);

	for(i=0;i<len;i++)
	{
		if (max[i]<'"')
			max[i] += 16 * 6;
		else
		if (max[i]>(16*6))
			max[i] -= 16 * 6;
	}

	Cvar_FullSet ( value, max ,0);
}



//================================================================
//dave - if-else stack
#define EXECUTE 1
#define NOIFBLOCK -2//the entire if block is embedded in a non-execute block
#define NOEXECUTE -1
int ifstack[100];
int ifstack_top = 0;
void If_Push(int value)
{
	if (ifstack_top == 100)
	{
		Con_Printf("doif: Stack overflow. The following execution will be erroneous.\n");
		return;
	}
	ifstack[ifstack_top] = value;
	ifstack_top++;
	Con_DPrintf("pushing %i - stack size %i\n", value, ifstack_top);
}
void If_Pop(void)
{
	if (ifstack_top < 1)
	{
		Con_Printf("doif: Stack underflow. Too many endifs?\n");
		return;
	}
	ifstack_top--;
	Con_DPrintf("popping - stack size %i\n", ifstack_top);
}
int If_Current(void)
{
	if(ifstack_top > 0)
	{
		return ifstack[ifstack_top-1];
	}
	return 0;//error!!!
}
void If_Current_Flip(void)
{
	Con_DPrintf("flipping from %ito %i\n", ifstack[ifstack_top-1], ifstack[ifstack_top-1]*-1);
	ifstack[ifstack_top-1] *= -1;
}
//dave - if command
void Cmd_If (void)
{
	float firstval, secondval;

	if (Cmd_Argc() != 4)
	{
		Con_Printf("doif val1 <|>|=|<=|>= val2 : your typical if statement\n");
		return;
	}

	firstval = Q_atof(Cmd_Argv(1));
	secondval = Q_atof(Cmd_Argv(3));

	if (!(strcmp(Cmd_Argv(2), "=")))
	{
		//equals - do a string comp
		if (!( strcmp(Cmd_Argv(1), Cmd_Argv(3)) ))
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else if (!(strcmp(Cmd_Argv(2), "==")))
	{
		//double equals - do a numerical comparison
		if (firstval == secondval)
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else if (!( strcmp(Cmd_Argv(2), "<") ))
	{
		//is less than - all these ones can assume numerical
		if (firstval < secondval)
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else if (!( strcmp(Cmd_Argv(2), ">") ))
	{
		//is greater than
		if (firstval > secondval)
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else if (!( strcmp(Cmd_Argv(2), "<=") ))
	{
		//less or equal
		if (firstval <= secondval)
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else if (!( strcmp(Cmd_Argv(2), ">=") ))
	{
		//greater or equal
		if (firstval >= secondval)
			If_Push(EXECUTE);
		else
			If_Push(NOEXECUTE);
	}
	else
	{
		Con_Printf("doif val1 <|>|=|<=|>=|== val2 : your typical if statement\n");
		return;
	}
}

void Cmd_If_Null (void)
{
	return;
}
//-dave
//================================================================


/*
============
Cmd_ExpandString
Taken from ZQuake - replaces $cvarname with the contents of the cvar
============
*/
// dest must point to a 1024-byte buffer
void Cmd_ExpandString (char *data, char *dest)
{
	unsigned int	c;
	char	buf[255];
	int		i, len;
	cvar_t	*var, *bestvar;
	int		quotes = 0;

	len = 0;

// parse a regular word
	while ( (c = *data) != 0)
	{
		if (c == '"')
			quotes++;
		if (c == '$' && !(quotes&1))
		{
			data++;

			// Copy the text after '$' to a temp buffer
			i = 0;
			buf[0] = 0;
			bestvar = NULL;
			while (((c = *data) <= 32) && (c != 0))
			{
				if (c == '$')
					break;
				data++;
				buf[i++] = c;
				buf[i] = 0;
				if ( (var = Cvar_FindVar(buf)) != NULL )
					bestvar = var;
			}

			if (bestvar)
			{
				// check buffer size
				if (len + strlen(bestvar->string) >= 1024-1)
					break;

				strcpy(&dest[len], bestvar->string);
				len += strlen(bestvar->string);
				i = strlen(bestvar->name);
				while (buf[i])
					dest[len++] = buf[i++];
			}
			else
			{
				// no matching cvar name was found
				dest[len++] = '$';
				if (len + strlen(buf) >= 1024-1)
					break;
				strcpy (&dest[len], buf);
				len += strlen(buf);
			}
		}
		else
		{
			dest[len] = c;
			data++;
			len++;
			if (len >= 1024-1)
				break;
		}
	};

	dest[len] = 0;
}



/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
} cmd_function_t;

#define	MAX_ALIAS_NAME	32
typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;


/*
================
Cmd_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/
/*
int Cmd_CheckParm (char *parm)
{
	int i;
	
	if (!parm)
		Sys_Error ("Cmd_CheckParm: NULL");

	for (i = 1; i < Cmd_Argc (); i++)
		if (! Q_strcasecmp (parm, Cmd_Argv (i)))
			return i;
			
	return 0;
}
*/

#define	MAX_ARGS		80
extern char * cmd_argv[MAX_ARGS];
extern cmd_function_t	*cmd_functions;	
extern cmdalias_t	*cmd_alias;

void	Cmd_ExecuteString (char *text, cmd_source_t src)
{	
	cmd_function_t	*cmd;
	cmdalias_t		*a;


	cmd_source = src;
	Cmd_TokenizeString (text);
			
// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

	//dave - else and endif 'command's
	if (ifstack_top > 0)
	{
		if(!Q_strcasecmp (cmd_argv[0],"endif"))
		{
			If_Pop();
			return;
		}
		else if(!Q_strcasecmp (cmd_argv[0],"else"))
		{
			if (If_Current() != NOIFBLOCK)
			{
				If_Current_Flip();
				return;
			}
			else
			{
				return;
			}
		}
		if ((If_Current() == NOEXECUTE) || (If_Current() == NOIFBLOCK))
		{
			if(!Q_strcasecmp (cmd_argv[0],"doif"))
			{
				If_Push(NOIFBLOCK);
			}
			return;
		}
	}
	//-dave


#if 1
// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcasecmp (cmd_argv[0],cmd->name))
		{
			cmd->function ();
			return;
		}
	}
#else
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcasecmp (cmd_argv[0],cmd->name))
		{
			if (!cmd->function)
				Cmd_ForwardToServer ();
			else
				cmd->function ();
			return;
		}
	}
#endif



// check alias
	for (a=cmd_alias ; a ; a=a->next)
	{
		if (!Q_strcasecmp (cmd_argv[0], a->name))
		{
			Cbuf_InsertText (a->value);
			return;
		}
	}

	
	if (!Cvar_Command () )
			Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
	
}



void Cmd_Null_f (void)
{
}


void Cmd_DEcho_f (void)
{
	int		i;

	for (i=1 ; i<Cmd_Argc() ; i++)
		Con_DPrintf ("%s ",Cmd_Argv(i));
	Con_DPrintf ("\n");
}

extern cvar_t temp2;

void Cmd_Eval_f (void)
{
	static int		i;
	static cvar_t * mach;
	char command[1024];

	if (Cmd_Argc()<3)
	{
		Con_Printf("eval <cvar mach with temp2> <expr1> <expr2> <expr3>...\n");
		return;
	}

	mach = Cvar_FindVar (Cmd_Argv(1));

	if (!mach)
	{
		Con_Printf("'%s' not found.", Cmd_Argv(1));
		return;
	}

	if (temp2.value == mach->value || !strcmp(temp2.string,mach->string))
	for (i=1 ; i<Cmd_Argc() ; i++)
	{
		sprintf(command,"%s;",Cmd_Argv(i));
		Cbuf_AddText (command);Cbuf_AddText ("\n");		
	}	
}

void Cmd_Bindefault_f (void)
{
	if (Cmd_Argc()!=2)
	{
		Con_Printf("usage: bindefault <default builtin profile>\n");
		Con_Printf("	know bindprofiles: menu, tei\n");
		return;
	}


	if (!strcmp("menu",Cmd_Argv(1)))
		Cbuf_AddText ("bind [ menu_up;bind ] menu_down;bind enter menu_exec\n");
	else
	if (!strcmp("tei",Cmd_Argv(1)))
		Cbuf_AddText ("bindefault menu;bind s \"impulse 10\";bind a +moveleft;bind f +moveright\n;sensitivity 44;bind SPACE +jump;bind MOUSE1 +forward;bind MOUSE2 +back;bind ` toggleconsole\n");
	else
		Con_Printf("unknow '%s' bindefault\n",Cmd_Argv(1));
}


/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init (void)
{

	Cmd_AddCommand ("#", Cmd_Null_f);//dave - comment command, so as not to complain.
	Cmd_AddCommand ("decho",Cmd_DEcho_f);//qblood
	//Cmd_AddCommand ("echo",Cmd_Echo_f);//qblood

	//Tei best commands
	Cmd_AddCommand ("set", Cvar_Set_f);// new cvar 
	Cmd_AddCommand ("dec", Cvar_Dec_f);// - 1
	Cmd_AddCommand ("mov", Cvar_Mov_f);// move var to var
	Cmd_AddCommand ("inc", Cvar_Inc_f);// + 1
	Cmd_AddCommand ("add", Cvar_AddVars_f);// add var to var
	Cmd_AddCommand ("mul", Cvar_MultVars_f);// add var to var

	Cmd_AddCommand ("bound", Cvar_BoundVar_f);//bound cvar to min/max
	Cmd_AddCommand ("concat", Cvar_ConCat_f);//concat strings

	Cmd_AddCommand ("link", Cvar_Link_f);//link cvars
	

	Cmd_AddCommand("accum",Cvar_Accum_f);// Bitset set
	Cmd_AddCommand("reset",Cvar_Reset_f);// Bitset clear

	Cmd_AddCommand("trigger",Cvar_Trigger_f);
	Cmd_AddCommand("triggerlist",Cvar_TriggerList_f);

	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("toggletext", Cvar_ToggleText_f);//error

	Cmd_AddCommand ("slider", Cvar_Slider_f);
	Cmd_AddCommand ("combobox", Cvar_ComboBox_f);
	
	Cmd_AddCommand ("cvarlist", Cvar_List_f);

	Cmd_AddCommand ("bindefault", Cmd_Bindefault_f);


	Cmd_AddCommand ("eval", Cmd_Eval_f);

	//Tei best commands

	//Cmd_AddCommand ("cmd", Cmd_ForwardToServer_f);//qblood
}


// Q2!


