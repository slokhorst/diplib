%MAKECONTENTS   Generates the Contents file for DIPimage.
%   Run this function from within the directory it is in:
%      cd <root>/diplib/tools
%      makecontents <outputdir>
%   It will delete any Contents.m file in 'outputdir',
%   and create a new one based off of the H-1 lines in all M-files,
%   using the groupings specified in DIPMENUS.

function makecontents(outputdir)

disp(['Creating Contents.m in ',outputdir])

% Get current date string
ds = date;

% Find DIPlib version number from ../CMakeLists.txt
f = fopen(fullfile('..','CMakeLists.txt'),'r');
if f<0
   error('Error opening "CMakeLists.txt"')
end
t1 = [];
t2 = [];
t3 = [];
cc = [];
while true
   l = fgetl(f);
   if feof(f)
      fclose(f);
      error('Couldn''t find complete version number in "CMakeLists.txt"')
   end
   if isempty(t1)
      t1 = regexp(l,'set\(PROJECT_VERSION_MAJOR "(.+)"\)','tokens');
      if ~isempty(t1)
         continue
      end
   elseif isempty(t2)
      t2 = regexp(l,'set\(PROJECT_VERSION_MINOR "(.+)"\)','tokens');
      if ~isempty(t2)
         continue
      end
   elseif isempty(t3)
      t3 = regexp(l,'set\(PROJECT_VERSION_PATCH "(.+)"\)','tokens');
  else
      cc = regexp(l,'add_definitions\(-DDIP_COPYRIGHT_YEAR="(.+)"\)','tokens');
      if ~isempty(cc)
         break % This component is written last in the CMakeLists.txt file
      end
   end
end
fclose(f);
vs = [t1{1}{1},'.',t2{1}{1},'.',t3{1}{1}];
cc = cc{1}{1};

% Load dipmenus
dipimage_dir = fullfile('..','dipimage');
addpath(dipimage_dir)
menulist = dipmenus;
menulist = [{'GUI',{'dipimage','dipshow','viewslice'}};...
            {'Configuration',{'dipgetpref','dipsetpref','dipinit','dipfig','dipmenus'}};...
            menulist];

% Find functions to list
mfiles = dir(fullfile(dipimage_dir,'*.m'));
dipviewer_dir = fullfile('..','viewer','dipimage');
mfiles2 = dir(fullfile(dipviewer_dir,'*.m'));
mfilenames = {mfiles.name,mfiles2.name};
mfiledirs = [repmat({dipimage_dir},1,numel(mfiles)),repmat({dipviewer_dir},1,numel(mfiles2))];
mfiles2 = [];
mfiles = [];
[mfilenames,I] = sort(lower(mfilenames));
mfiledirs = mfiledirs(I);
I = strncmp('dip_',mfilenames,4);
mfilenames(I) = {''};

% Create Contents.m
outputfile = fullfile(outputdir,'Contents.m');
f = fopen(outputfile,'w');
if f<0
   error(['Error opening "',outputfile,'"'])
end
fprintf(f,'%% DIPimage toolbox for quantitative image analysis\n');
fprintf(f,'%% Version %s   %s\n',vs,ds);
fprintf(f,'%% (c)2016-%s, Cris Luengo and contributors\n',cc);
fprintf(f,'%% (c)1999-2014, Delft University of Technology\n');
fprintf(f,'%%\n');

% Formatting for function list
l = max(cellfun(@length,mfilenames));
l = max(l,16);
format = ['%%   %-',num2str(l),'s - %s\n'];

% Write the functions in the menulist next
for ii = 1:size(menulist,1)
   functionlist = menulist{ii,2}(:).'; % reshape just in case...
   valid = false(size(functionlist));
   for jj=1:size(functionlist,2)
      if ~strcmp(functionlist{1,jj},'-') && functionlist{1,jj}(1)~='#'
         I = strcmp([functionlist{1,jj},'.m'],mfilenames);
         if any(I)
            mfilenames(I) = {''};
            functionlist{2,jj} = getdescription(functionlist{1,jj},mfiledirs{I});
            valid(jj) = true;
         end
      end
   end
   if any(valid)
      fprintf(f,'%% %s:\n',menulist{ii,1});
      fprintf(f,format,functionlist{:,valid});
      fprintf(f,'%%\n');
   end
end

% Write the functions related to the image display
fprintf(f,'%% Interactive image display:\n');
descr = getdescription('dipshow',dipimage_dir);
fprintf(f,format,'dipshow',descr); % we repeat this one here
descr = getdescription('diptruesize',dipimage_dir);
fprintf(f,format,'diptruesize',descr); % we repeat this one here
descr = getdescription('dipclf',dipimage_dir);
fprintf(f,format,'dipclf',descr); % we repeat this one here
for ii = 1:length(mfilenames)
   if strncmp('dip',mfilenames{ii},3)
      func = mfilenames{ii}(1:end-2);
      descr = getdescription(func,mfiledirs{ii});
      fprintf(f,format,func,descr);
      mfilenames{ii} = '';
   end
end
fprintf(f,'%%\n');

% Write the rest of the functions
fprintf(f,'%% Other available functions are:\n');
for ii = 1:length(mfilenames)
   if ~isempty(mfilenames{ii})
      func = mfilenames{ii}(1:end-2);
      descr = getdescription(func,mfiledirs{ii});
      fprintf(f,format,func,descr);
   end
end
fprintf(f,'%%\n');

% Footer
fprintf(f,'%% Type\n');
fprintf(f,'%%   methods dip_image\n');
fprintf(f,'%% to get a list of functions overloaded for dip_images.\n');
fprintf(f,'%%\n');
fprintf(f,'%% More information is available in the DIPimage User Manual, type\n');
fprintf(f,'%%   web(dipgetpref(''UserManualLocation''),''-browser'')\n');
fprintf(f,'\n');
fprintf(f,'%% (c)2017-%s, Cris Luengo.\n',cc);
fprintf(f,'%%\n');
fprintf(f,'%% Licensed under the Apache License, Version 2.0 (the "License");\n');
fprintf(f,'%% you may not use this file except in compliance with the License.\n');
fprintf(f,'%% You may obtain a copy of the License at\n');
fprintf(f,'%%\n');
fprintf(f,'%%    http://www.apache.org/licenses/LICENSE-2.0\n');
fprintf(f,'%%\n');
fprintf(f,'%% Unless required by applicable law or agreed to in writing, software\n');
fprintf(f,'%% distributed under the License is distributed on an "AS IS" BASIS,\n');
fprintf(f,'%% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n');
fprintf(f,'%% See the License for the specific language governing permissions and\n');
fprintf(f,'%% limitations under the License.\n');
fclose(f);

end

%%%
function descr = getdescription(funcname,dirname)
descr = '';
f = fopen(fullfile(dirname,[funcname,'.m']),'r');
if f < 0
   return
end
while isempty(descr)
   descr = fgetl(f);
   if feof(f)
      fclose(f);
      descr = '';
      return
   end
end
fclose(f);
tokens = regexp(descr,'^\s*%\s*(\S*)\s*(.*)','tokens','once','dotexceptnewline');
if isempty(tokens)
   descr = '';
else
   if ~strcmp(tokens{1},upper(funcname))
      warning([funcname,' doesn''t report the right name in the H-1 line'])
   end
   descr = tokens{2};
end
end
