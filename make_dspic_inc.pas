{   Program MAKE_DSPIC_INC [pic]
*
*   Make the include file for each dsPIC processor that has a linker file, or
*   for a single specific processor.  The include file will contain definitions
*   of symbols found in the linker file that meet the following criteria:
*
*     1 - The name starts with an alphabetic character.
*
*     2 - All alphabetic characters in the symbol are upper case.
*
*     3 - The symbol is assigned a literal integer constant.
*
*   These symbols will be defined in the output file with .equiv statements.
*   The symbol names in the output file will have the first character upper
*   case, with all following aphabetic characters, if any, lower case.
*
*   With the PIC command line option omitted, all linker files in the current
*   dsPIC tools installation will be processed and an include file created for
*   each of them.
*
*   If the PIC command line option is provided, then only the file for that
*   PIC will be created.  PIC must be the model number without the preceeding
*   "PIC".  For example, "30F3013" and "24FJ256DA210".  The PIC command line
*   option is case-insensitive.
}
program make_dspic_inc;
%include 'base.ins.pas';

const
  max_msg_args = 2;                    {max arguments we can pass to a message}

var
  onlypic:                             {only create include file for this PIC}
    %include '(cog)lib/string32.ins.pas';
  npics: sys_int_machine_t;            {number of PICs include file written for}
  tnam:                                {scratch treename}
    %include '(cog)lib/string_treename.ins.pas';
  stat: sys_err_t;                     {completion status}
{
********************************************************************************
*
*   Subroutine DO_DIR (DIR)
*
*   Process all the linker files, if any, in the directory tree DIR.  This
*   routine calls itself recursively for any subdirectories.
*
*   If the global string ONLYPIC is not empty, then only the include file for
*   that PIC will be created.
*
*   The global variable NPICS is incremented by 1 for each include file created.
}
procedure do_dir (                     {process GLD files from one directory}
  in      dir: string_treename_t);     {the directory to process .gld files from}
  val_param; internal;

var
  conn_dir: file_conn_t;               {connection to the directory to scan}
  conn_in: file_conn_t;                {connection to linker input file}
  conn_out: file_conn_t;               {connection to output include file}
  p: string_index_t;                   {parse index}
  ii: sys_int_machine_t;               {scratch integer and loop counter}
  finfo: file_info_t;                  {info about directory entry}
  radix: sys_int_machine_t;            {radix of integer digits string}
  ival: sys_int_max_t;                 {symbol integer value}
  time: sys_clock_t;                   {scratch time descriptor}
  c: char;                             {scratch character}

  ibuf: string_var132_t;               {one line input buffer}
  obuf: string_var132_t;               {one line output buffer}
  tnam, tnam2: string_treename_t;      {scratch treenames}
  fnam: string_leafname_t;             {current directory entry name}
  gnam: string_leafname_t;             {generic file name}
  tk: string_var80_t;                  {scratch string or token}
  name: string_var80_t;                {symbol name}
  sval: string_var80_t;                {symbol's string value}
  incdir: string_treename_t;           {treename of directory containing .INC files}
  incname: string_treename_t;          {treename of include file for current linker file}
  picname: string_var32_t;             {upper case target PIC name, like "30F3013"}
  picfam: string_var32_t;              {lower case dsPIC family name, like 30f, 24h, etc}

  msg_parm:                            {references arguments passed to a message}
    array[1..max_msg_args] of sys_parm_msg_t;
  stat: sys_err_t;                     {completion status}

label
  loop_fnam, have_digit, loop_iline, have_ival, eof;
{
****************************************
*
*   Local Subroutine WBUF
*
*   Write the contents of OBUF to the output file as the next line and reset
*   OBUF to empty.
}
procedure wbuf;                        {write OBUF to output file, clear OBUF}

var
  stat: sys_err_t;

begin
  file_write_text (obuf, conn_out, stat); {write the output line}
  sys_error_abort (stat, '', '', nil, 0);
  obuf.len := 0;                       {reset the output buffer to empty}
  end;
{
****************************************
*
*   Start of executable code of DO_DIR.
}
begin
  ibuf.max := size_char(ibuf.str);     {init local var strings}
  obuf.max := size_char(obuf.str);
  tnam.max := size_char(tnam.str);
  tnam2.max := size_char(tnam2.str);
  fnam.max := size_char(fnam.str);
  gnam.max := size_char(gnam.str);
  tk.max := size_char(tk.str);
  name.max := size_char(name.str);
  sval.max := size_char(sval.str);
  incdir.max := size_char(incdir.str);
  incname.max := size_char(incname.str);
  picname.max := size_char(picname.str);
  picfam.max := size_char(picfam.str);
{
*   Open the directory for reading its entries.
}
  file_open_read_dir (dir, conn_dir, stat);
  sys_msg_parm_vstr (msg_parm[1], dir);
  sys_error_abort (stat, 'pic', 'err_dspic_linkdir', msg_parm, 1);
{
*   Read and process each directory entry.  If the entry is a file name ending in
*   ".gld", then process it as a linker file.  If the entry is a directory then
*   process it recursively.
}
loop_fnam:                             {back here each new linker files directory entry}
  file_read_dir (                      {get next linker files directory entry}
    conn_dir,                          {connection to the directory}
    [file_iflag_dtm_k, file_iflag_type_k], {info requested about this directory entry}
    fnam,                              {returned directory entry name}
    finfo,                             {info about this directory entry}
    stat);
  if file_eof(stat) then begin         {hit end of directory ?}
    file_close (conn_dir);             {close the directory}
    return;                            {all done}
    end;
  sys_error_abort (stat, '', '', nil, 0);

  case finfo.ftype of

file_type_data_k: ;                    {fall thru to process files or links}
file_type_link_k: ;

file_type_dir_k: begin                 {this directory entry is a subdirectory}
      string_copy (conn_dir.tnam, tnam); {make full subdirectory pathname}
      string_append1 (tnam, '/');
      string_append (tnam, fnam);
      do_dir (tnam);                   {recursively process this subdirectory}
      goto loop_fnam;
      end;

otherwise
    goto loop_fnam;                    {not a type we can handle, ignore this entry}
    end;

  if fnam.len < 5 then goto loop_fnam; {can't be at least x.gld file name ?}
  string_substr (                      {extract last 4 characters of file name}
    fnam,                              {input string}
    fnam.len - 3,                      {start index to extract from}
    fnam.len,                          {end index to extract from}
    gnam);                             {extracted string}
  string_downcase (gnam);
  if not string_equal (gnam, string_v('.gld')) {not a linker script file name ?}
    then goto loop_fnam;
{
*   This directory entry is a .gld file.
*
*   Extract the PIC name in upper case into PICNAME.  A example PIC name is
*   30F2010.
}
  for ii := 1 to fnam.len do begin     {scan generic name looking for first digit}
    c := fnam.str[ii];                 {fetch this character}
    if (c >= '0') and (c <= '9') then goto have_digit;
    end;
  sys_msg_parm_vstr (msg_parm[1], fnam);
  sys_message_parms ('pic', 'err_picname', msg_parm, 1);
  goto loop_fnam;
have_digit:                            {II is index of first digit in generic input fnam}
  string_substr (fnam, ii, fnam.len-4, picname); {extract the PIC name}
  string_upcase (picname);             {save PIC name all upper case}
{
*   Skip this file if a specific PIC name was supplied and it does not match
*   this file.
}
  if onlypic.len > 0 then begin        {only make file for specific PIC ?}
    if not string_equal(picname, onlypic) then goto loop_fnam; {not the right PIC ?}
    end;
{
*   Make the PIC family name lower case in PICFAM.  This is the family number
*   followed by the first letter, like 24H and 33F.
}
  for ii := 1 to picname.len do begin  {scan looking for first non-digit of PIC name}
    c := picname.str[ii];              {get this character}
    if (c < '0') or (c > '9') then exit; {found first non-digit ?}
    end;
  ii := min(ii, picname.len);          {make sure last char is within string}
  string_substr (picname, 1, ii, picfam); {extract the PIC family designator}
  string_downcase (picfam);            {make final PIC family name in lower case}
{
*   Set INCNAME to the full pathname of the corresponding Microchip include file
*   for this PIC.
}
  string_copy (conn_dir.tnam, tnam);   {init to this directory name}
  string_appends (tnam, '/../inc/'(0));
  string_append (tnam, fnam);
  tnam.len := tnam.len - 4;            {remove .gld linker file name suffix}
  string_appends (tnam, '.inc'(0));
  string_treename (tnam, incname);     {make full pathname of the include file}
  if not file_exists (incname)         {skip this GLD file if include file not exist}
    then goto loop_fnam;
{
*   Open this linker file for read on CONN_IN.
}
  string_pathname_join (conn_dir.tnam, fnam, tnam2);
  string_treename (tnam2, tnam);       {make full treename of this linker file}
  file_open_read_text (tnam, '.gld', conn_in, stat); {open linker file for read}
  sys_error_abort (stat, '', '', nil, 0);
{
*   Open the output file to create for this linker file.
}
  file_open_write_text (conn_in.gnam, '.ins.dspic', conn_out, stat); {open output file}
  sys_error_abort (stat, '', '', nil, 0);
  time := sys_clock;                   {get time output file created}
  writeln (conn_out.tnam.str:conn_out.tnam.len); {show name of output file being created}
{
*   A linker script is open on CONN_IN, and the assembler include file to create from
*   it is open on CONN_OUT.
}
  obuf.len := 0;                       {init output buffer to empty}
  string_appends (obuf, ';   Derived from linker script "');
  string_append (obuf, conn_in.tnam);
  string_appends (obuf, '",');
  wbuf;
  string_appends (obuf, ';   which was last changed at ');
  sys_clock_str1 (finfo.modified, tk); {make linker file last modified date/time string}
  string_append (obuf, tk);
  string_append1 (obuf, '.');
  wbuf;
  string_appends (obuf, ';');
  wbuf;
  string_appends (obuf, ';   Created by program MAKE_DSPIC_INC at ');
  sys_clock_str1 (time, tk);
  string_append (obuf, tk);
  string_appends (obuf, ' on machine ');
  sys_node_name (tk);
  string_upcase (tk);
  string_append (obuf, tk);
  string_append1 (obuf, '.');
  wbuf;
  string_appends (obuf, ';');
  wbuf;

  string_appends (obuf, '/var new Picname string');
  wbuf;
  string_appends (obuf, '/set Picname "');
  string_append (obuf, picname);
  string_appends (obuf, '"');
  wbuf;

  string_appends (obuf, '.ifndef __');
  string_append (obuf, picname);
  wbuf;
  string_appends (obuf, ' .equiv __');
  string_append (obuf, picname);
  string_appends (obuf, ', 1');
  wbuf;
  string_appends (obuf, ' .endif');
  wbuf;

  string_appends (obuf, '/include "');
  string_append (obuf, incname);
  string_appends (obuf, '"');
  wbuf;
  wbuf;

loop_iline:                            {back here each new linker file input line}
  file_read_text (conn_in, ibuf, stat); {read next line from linker file}
  if file_eof(stat) then goto eof;     {end of file ?}
  sys_error_abort (stat, '', '', nil, 0);
  for ii := 1 to ibuf.len do begin     {convert all control characters to spaces}
    if ord(ibuf.str[ii]) < 32 then ibuf.str[ii] := ' ';
    end;
  string_unpad (ibuf);                 {truncate trailing spaces}
  if ibuf.len <= 0 then goto loop_iline; {ignore blank lines}
  p := 1;                              {init input line parse index}
{
*   Get and validate the name of the symbol being defined.  The symbol name
*   converted to the format used in the output file will be left in NAME.
}
  string_token_anyd (                  {try to extract name symbol token before "="}
    ibuf,                              {input string}
    p,                                 {parse index}
    ' =', 2,                           {list of delimiters}
    1,                                 {first N delimiters that may be repeated}
    [string_tkopt_padsp_k],            {strip leading/trailing blank padding around token}
    name,                              {the extracted symbol name}
    ii,                                {index of main ending delimiter}
    stat);
  if sys_error(stat) then goto loop_iline; {ignore line on parsing error}
  if ii <> 2 then goto loop_iline;     {token wasn't followed by equal sign ?}
  if name.len <= 0 then goto loop_iline; {symbol name token is empty ?}
  if (name.str[1] < 'A') or (name.str[1] > 'Z') {not start with upper case alphabetic char ?}
    then goto loop_iline;
  for ii := 2 to name.len do begin     {scan remaining characters after the first}
    if (name.str[ii] >= 'a') and (name.str[ii] <= 'z') {lower case alphabetic character ?}
      then goto loop_iline;
    name.str[ii] := string_downcase_char (name.str[ii]); {downcase to make output name}
    end;
{
*   Get the value being assinged to the symbol.
}
  string_token_anyd (                  {try to extract symbol value between "=" and ";"}
    ibuf,                              {input string}
    p,                                 {parse index}
    ' ;', 2,                           {list of delimiters}
    1,                                 {first N delimiters that may be repeated}
    [string_tkopt_padsp_k],            {strip leading/trailing blank padding around token}
    sval,                              {the extracted symbol value string}
    ii,                                {index of main ending delimiter}
    stat);
  if sys_error(stat) then goto loop_iline; {ignore line on parsing error}
  if ii <> 2 then goto loop_iline;     {token wasn't followed by semicolon ?}
  if sval.len <= 0 then goto loop_iline; {symbol value string is empty ?}
  string_upcase (sval);                {make value string all upper case}

  if sval.str[1] <> '0' then begin     {normal decimal integer ?}
    string_t_int_max (sval, ival, stat);
    if sys_error(stat) then goto loop_iline; {not a valid integer ?}
    goto have_ival;
    end;

  case sval.str[2] of                  {what follows leading zero character ?}
'B': begin                             {binary}
      radix := 2;                      {radix}
      ii := 3;                         {start of integer index}
      end;
'0', '1', '2', '3', '4', '5', '6', '7': begin {octal}
      radix := 8;                      {radix}
      ii := 2;                         {start of integer index}
      end;
'X': begin                             {hexadecimal}
      radix := 16;                     {radix}
      ii := 3;                         {start of integer index}
      end;
otherwise
    goto loop_iline;                   {unexpected radix indicator, ignore line}
    end;
  if sval.len < ii then goto loop_iline; {string too short to be valid integer ?}
  string_substr (sval, ii, sval.len, tk); {extract just the integer digits into TK}
  string_t_int_max_base (              {convert digits string to integer value}
    tk,                                {input string}
    radix,                             {radix}
    [string_ti_unsig_k],               {convert digits as unsigned value}
    ival,                              {resulting integer value}
    stat);
  if sys_error(stat) then goto loop_iline; {not a real integer value, ignore line ?}
have_ival:                             {symbol value is in IVAL}
{
*   A valid symbol assignment was parsed from the current input line.  The
*   symbol name in output file format is in NAME, and its integer value is
*   in IVAL.
*
*   Now write the definition of this symbol to the output file.
}
  string_appends (obuf, '.equiv ');
  string_append (obuf, name);
  string_appends (obuf, ', 0x');
  string_f_int_max_base (              {make hexadecimal string of integer value}
    tk,                                {output string}
    ival,                              {input integer}
    16,                                {radix}
    0,                                 {free form field width}
    [string_fi_unsig_k],               {assume input integer is unsigned}
    stat);
  sys_error_abort (stat, '', '', nil, 0);
  string_append (obuf, tk);
  wbuf;                                {write this line to the output file}
  goto loop_iline;                     {back for next input file line}

eof:                                   {end of linker input file encountered}
  file_close (conn_in);                {close the linker input file}

  wbuf;
  string_appends (obuf, '/include "lcase.ins.dspic"');
  wbuf;

  file_close (conn_out);               {close the output file}
  npics := npics + 1;                  {count one more include file created}
  goto loop_fnam;                      {back to try next directory entry}
  end;
{
********************************************************************************
*
*   Start of main routine.
}
begin
  string_cmline_init;                  {init for reading the command line}
  string_cmline_token (onlypic, stat); {try to get single PIC to make file for}
  if string_eos(stat)
    then begin                         {end of command line}
      onlypic.len := 0;                {indicate make include file for all PICs}
      end
    else begin                         {got single PIC name or hard error}
      sys_error_abort (stat, '', '', nil, 0); {abort on hard error}
      string_upcase (onlypic);         {command line arugment can be any case}
      string_cmline_end_abort;         {no more command line arguments allowed}
      end
    ;
  npics := 0;                          {init number of include files created}

  string_vstring (                     {set top of tree to search}
    tnam, '(cog)extern/mplab/support16'(0), -1);
  do_dir (tnam);                       {scan all linker files in the tree}

  if onlypic.len > 0
    then begin                         {creating file for a specific PIC only}
      if npics <= 0 then begin         {nothing created ?}
        writeln ('No files found for the indicated PIC.');
        sys_bomb;
        end;
      end
    else begin                         {created files for all PICs}
      writeln (npics, ' files created');
      end
    ;
  end.
