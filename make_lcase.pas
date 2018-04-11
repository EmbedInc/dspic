{   Program MAKE_LCASE
*
*   One-off hack to create the LCASE.INS.DSPIC dsPIC assembler include file.
*   This file creates lower case symbols for all the standard symbols created
*   in the Microchip include file.  A lower case version of a symbol is only
*   created if the upper case version of it previously exists.
*
*   The input file is assumed to be "ucase" in the current directory.  It
*   contains the symbols from the Microchip include file in the case they
*   are defined in, one per line.  Leading and trailing spaces are allowed.
*
*   For each input symbol, if the symbol is not already all lower case
*   with the first letter upper case, the appropriate assembler statements
*   are written so that such a symbol is created if the symbol with the
*   original case already exists.
*
*   The output file is "lcase.ins.dspic" and will be written to the
*   current directory.
}
program make_lcase;
%include '(cog)lib/sys.ins.pas';
%include '(cog)lib/util.ins.pas';
%include '(cog)lib/string.ins.pas';
%include '(cog)lib/file.ins.pas';

var
  conn_in, conn_out: file_conn_t;      {input and output file connections}
  p: string_index_t;                   {parse index}
  ibuf:                                {one line input buffer}
    %include '(cog)lib/string80.ins.pas';
  obuf:                                {one line output buffer}
    %include '(cog)lib/string80.ins.pas';
  symu, syml:                          {upper and lower case symbol names}
    %include '(cog)lib/string80.ins.pas';
  stat: sys_err_t;                     {completion status}

label
  loop_iline, eof;

begin
  file_open_read_text (                {open the input file}
    string_v('ucase'), '',             {file name and suffix}
    conn_in,                           {returned connection to the file}
    stat);
  sys_error_abort (stat, '', '', nil, 0);

  file_open_write_text (               {open the output file}
    string_v('lcase.ins.dspic'), '',   {file name and suffix}
    conn_out,                          {returned connection to the output file}
    stat);
  sys_error_abort (stat, '', '', nil, 0);

loop_iline:                            {back here each new input line}
  file_read_text (conn_in, ibuf, stat); {read next line from input file}
  if file_eof(stat) then goto eof;     {hit end of input file ?}
  sys_error_abort (stat, '', '', nil, 0);

  string_unpad (ibuf);                 {strip trailing blanks}
  p := 1;                              {init input line parse index}
  string_token (ibuf, p, symu, stat);  {extract the input symbol name into SYMU}
  if string_eos(stat) then goto loop_iline; {empty line, ignore and go on to next ?}
  sys_error_abort (stat, '', '', nil, 0);
  string_token (ibuf, p, syml, stat);  {try to get another token}
  if not string_eos(stat) then begin   {not just end of string error ?}
    sys_error_abort (stat, '', '', nil, 0);
    writeln ('Extra crap on line ', conn_in.lnum);
    sys_bomb;
    end;
  if symu.len <= 0 then goto loop_iline; {empty token, ignore this line ?}
{
*   The next input symbol name to process is in SYMU.
}
  if symu.str[1] = '_'
    then begin                         {upper case name starts with underscore}
      string_substr (symu, 2, symu.len, syml); {extract name after underscore}
      end
    else begin                         {no leading underscore}
      string_copy (symu, syml);        {make lower case version of the symbol}
      end
    ;
  string_downcase (syml);
  syml.str[1] := string_upcase_char (syml.str[1]); {upcase only the first char}
  if string_equal (syml, symu)         {desired symbol already exists ?}
    then goto loop_iline;
{
*   The original symbol name is in SYMU and the all lower case version of it
*   is in SYML, and the two are guaranteed to be different.
*
*   Now write the assembler directives to create the lower case symbol equal
*   to the upper case one if the upper case one already exists.  The following
*   directives will be written to the output file.
*
*   .ifndef <syml>
*     .ifdef <symu>
*       .equiv <syml>, <symu>
*       .endif
*     .endif
}
  string_vstring (obuf, '.ifndef '(0), -1);
  string_append (obuf, syml);
  file_write_text (obuf, conn_out, stat); sys_error_abort (stat, '', '', nil, 0);

  string_vstring (obuf, '  .ifdef '(0), -1);
  string_append (obuf, symu);
  file_write_text (obuf, conn_out, stat); sys_error_abort (stat, '', '', nil, 0);

  string_vstring (obuf, '    .equiv '(0), -1);
  string_append (obuf, syml);
  string_appends (obuf, ', '(0));
  string_append (obuf, symu);
  file_write_text (obuf, conn_out, stat); sys_error_abort (stat, '', '', nil, 0);

  string_vstring (obuf, '    .endif'(0), -1);
  file_write_text (obuf, conn_out, stat); sys_error_abort (stat, '', '', nil, 0);
  string_vstring (obuf, '  .endif'(0), -1);
  file_write_text (obuf, conn_out, stat); sys_error_abort (stat, '', '', nil, 0);
  goto loop_iline;
{
*   end of input file has been encountered.
}
eof:
  file_close (conn_in);
  file_close (conn_out);
  end.
