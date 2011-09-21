#------------------------------------------------------------------------
# (The MIT License)
# 
# Copyright (c) 2008-2011 Rhomobile, Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# http://rhomobile.com
#------------------------------------------------------------------------

#USE_TRACES = Rake.application.options.trace
USE_OWN_STLPORT = false


$current_platform = 'android'
$min_api_level = 4

if RUBY_PLATFORM =~ /(win|w)32$/
  $all_files_mask = "*.*"
  $rubypath = "res/build-tools/RhoRuby.exe"
  $bat_ext = ".bat"
  $exe_ext = ".exe"
  $path_separator = ";"
else
  $all_files_mask = "*"
  $bat_ext = ""
  $exe_ext = ""
  $path_separator = ":"
  if RUBY_PLATFORM =~ /darwin/
    $rubypath = "res/build-tools/RubyMac"
  else
    $rubypath = "res/build-tools/rubylinux"
  end
end
$basepath = File.expand_path File.dirname(__FILE__)
$androidpath = File.join $rootpath, "platform", "android"

require File.join($basepath, 'android_sdk.rb')

CONFIG = 'debug'
$buildroot = $appconfig['android']["build_path"]
BUILDPATH = File.join($buildroot, CONFIG)

BUILDARGS = {
              # Steps to ge-DHAVE_CONFIG_Ht curl_config.h from fresh libcurl sources:
              #export PATH=<ndkroot>/build/prebuilt/linux-x86/arm-eabi-4.2.1/bin:$PATH
              #export CC=arm-eabi-gcc
              #export CPP=arm-eabi-cpp
              #export CFLAGS="--sysroot <ndkroot>/build/platforms/android-3/arch-arm -fPIC -mandroid -DANDROID -DOS_ANDROID"
              #export CPPFLAGS="--sysroot <ndkroot>/build/platforms/android-3/arch-arm -fPIC -mandroid -DANDROID -DOS_ANDROID"
              #./configure --without-ssl --without-ca-bundle --without-ca-path --without-libssh2 --without-libidn --disable-ldap --disable-ldaps --host=arm-eabi
              "librhocommon" => ["-DRHO_NO_RUBY",
                           "-I#{$sharedpath}",
                           "-I#{File.join($sharedpath, "curl", "include")}"],
              "librhodb" =>  ["-DRHO_NO_RUBY",
                           "-I#{File.join($sharedpath, "db")}",
                           "-I#{$sharedpath}",
                           "-I#{File.join($sharedpath, "sqlite")}"],
              "libsync" => ["-DRHO_NO_RUBY",
                           "-I#{File.join($sharedpath, "sync")}",
                           "-I#{$sharedpath}",
                           "-I#{File.join($sharedpath, "sqlite")}"],
              "librhoimpl" => ["-DRHO_NO_RUBY",
                           "-I#{File.join($androidpath, "Rhodes", "jni", "include")}",
                           "-I#{File.join("Java", "RhoConnect", "jni", "include")}",
                           "-I#{File.join($sharedpath, "curl", "include")}",
                           "-I#{File.join($sharedpath, "common")}",
                           "-I#{File.join($sharedpath, "sqlite")}",
                           "-I#{$sharedpath}"],
              "rhoconnectclient" => ["-DRHO_NO_RUBY",
                           "-I#{File.join("Java", "RhoConnect", "jni", "include")}",
                           "-I#{$sharedpath}",
                           "-I#{File.join($androidpath, "Rhodes", "jni", "include")}"],
              "libcurl" =>   ["-DHAVE_CONFIG_H",
                           "-I#{File.join($sharedpath, "curl", "include")}",
                           "-I#{File.join($sharedpath, "curl")}",
                           "-I#{$sharedpath}"],
              "libsqlite" => ["-I#{File.join($sharedpath, "sqlite")}",
                           "-I#{$sharedpath}"],
              "libjson" =>   ["-DRHO_NO_RUBY",
                           "-I#{File.join($sharedpath, "json")}",
                           "-I#{$sharedpath}"],
              "libunzip" =>  ["-I#{$sharedpath}"],
              "librholog" => ["-DRHO_NO_RUBY",
                           "-I#{$sharedpath}"]
             }


SRC = FileList.new
OBJ = FileList.new
LIBFN = FileList.new
CLEAN = FileList.new

LIBS = Hash.new

LIBS['libcurl']   = File.join(BUILDPATH, 'libcurl.a')
LIBS['libsqlite'] = File.join(BUILDPATH, 'libsqlite.a')
LIBS['libjson']   = File.join(BUILDPATH, 'libjson.a')
LIBS['libunzip']  = File.join(BUILDPATH, 'libunzip.a')
LIBS['librholog'] = File.join(BUILDPATH, 'librholog.a')
LIBS['librhocommon'] = File.join(BUILDPATH, 'librhocommon.a')
LIBS['librhodb']  = File.join(BUILDPATH, 'librhodb.a')
LIBS['libsync']  = File.join(BUILDPATH, 'libsync.a')
LIBS['librhoimpl']  = File.join(BUILDPATH, 'librhoimpl.a')

CPPLIBS = ['libjson', 'librholog', 'librhocommon', 'librhodb', 'libsync', 'librhoimpl', 'rhoconnectclient']

LIBS.each do |name, filename|
  sources = get_sources(name)
  SRC.include sources
  OBJ.include get_objects(sources, File.join(BUILDPATH, name))
  LIBFN.include filename
end

librhoconnectclient_src = get_sources "rhoconnectclient"
SRC.include librhoconnectclient_src
OBJ.include get_objects(librhoconnectclient_src, File.join(BUILDPATH, "rhoconnectclient"))

CLEAN.include OBJ
CLEAN.include LIBFN
CLEAN.include File.join(BUILDPATH, 'librhoconnectclient.so')
CLEAN.include File.join(BUILDPATH, 'rhoimpl.jar')

namespace "android" do

  task :default => [File.join(BUILDPATH, 'librhoconnectclient.so'), File.join(BUILDPATH, "rhoimpl.jar")]

  namespace "config" do

    def fill_obj_src_lib name, buildpath
        libsources = get_sources name
        libsources.each do |src|
          objpath = get_object src, File.join(buildpath, name)
          $obj_src[objpath] = src
          $obj_lib[objpath] = name

          if USE_TRACES          
            puts "#{objpath} => #{$obj_src[objpath]}"
            puts "#{objpath} => #{$obj_lib[objpath]}"
            puts ""
          end
        end
    end

    def build_tree(buildpath)
      $obj_src = Hash.new
      $obj_lib = Hash.new
      LIBS.each do |name, filename|
        fill_obj_src_lib name, buildpath
      end
      fill_obj_src_lib "rhoconnectclient", buildpath
    end

    def build_args()
      CPPLIBS.each do |lib|
        unless $std_includes.nil?
          puts "Use stl for #{lib}" if USE_TRACES
          BUILDARGS[lib] << "-I#{$std_includes}"
        end
        if USE_OWN_STLPORT
          puts "Use own stlport for #{lib}" if USE_TRACES
          BUILDARGS[lib] << "-D__NEW__"
          BUILDARGS[lib] << "-I#{$stlport_includes}"
        end
      end
    end

    task :sdk do # => ["config:buildyml", "config:rhobuildyml"] do
      $androidsdkpath = $rhoconfig["env"]["paths"]["android"]
      sdk_level = setup_sdk $androidsdkpath, $min_api_level

      $androidndkpath = $rhoconfig["env"]["paths"]["android-ndk"]
      setup_ndk $androidndkpath, sdk_level
    end

    task :stdlib => [:sdk] do
      $std_includes = File.join $androidndkpath, "sources", "cxx-stl", "stlport", "stlport"
      unless File.directory? $std_includes
        $stlport_includes = File.join $sharedpath, "stlport", "stlport"
        USE_OWN_STLPORT = true
      end
    end

  end # namespace "config"

  task :config => ["config:sdk", "config:stdlib"] do
    build_tree BUILDPATH
    build_args
  end # task :config

  namespace "build" do

    task :libraries => [:libcurl, :libsqlite, :libjson, :libunzip, :librholog, :librhocommon, :librhodb, :libsync, :librhoimpl]

    task :libsqlite => [File.join(BUILDPATH, 'libsqlite'), LIBS['libsqlite'] ]

    task :libcurl => [File.join(BUILDPATH, 'libcurl'), LIBS['libcurl'] ]

    task :libjson => [File.join(BUILDPATH, 'libjson'), LIBS['libjson'] ]

    task :libunzip => [File.join(BUILDPATH, 'libunzip'), LIBS['libunzip'] ]

    task :librholog => [File.join(BUILDPATH, 'librholog'), LIBS['librholog'] ]

    task :librhocommon => [File.join(BUILDPATH, 'librhocommon'), LIBS['librhocommon'] ]

    task :librhodb => [File.join(BUILDPATH, 'librhodb'), LIBS['librhodb'] ]

    task :libsync => [File.join(BUILDPATH, 'libsync'), LIBS['libsync'] ]

    task :librhoimpl => [File.join(BUILDPATH, 'librhoimpl'), LIBS['librhoimpl'] ]

    task :rhoconnectclient => [File.join(BUILDPATH, 'librhoconnectclient.so')]

  end # namespace "build"

  task :clean do
    CLEAN.each do |filename|
      `rm -rf #{filename}`
    end
  end
end # namespace "android"

    directory BUILDPATH
    file BUILDPATH => ["android:config"]
    
    directory File.join(BUILDPATH, 'libcurl')
    file File.join(BUILDPATH, 'libcurl') => BUILDPATH

    directory File.join(BUILDPATH, 'libsqlite')
    file File.join(BUILDPATH, 'libsqlite') => BUILDPATH

    directory File.join(BUILDPATH, 'libjson')
    file File.join(BUILDPATH, 'libjson') => BUILDPATH

    directory File.join(BUILDPATH, 'libunzip')
    file File.join(BUILDPATH, 'libunzip') => BUILDPATH

    directory File.join(BUILDPATH, 'librholog')
    file File.join(BUILDPATH, 'librholog') => BUILDPATH

    directory File.join(BUILDPATH, 'librhocommon')
    file File.join(BUILDPATH, 'librhocommon') => BUILDPATH

    directory File.join(BUILDPATH, 'librhodb')
    file File.join(BUILDPATH, 'librhodb') => BUILDPATH

    directory File.join(BUILDPATH, 'libsync')
    file File.join(BUILDPATH, 'libsync') => BUILDPATH

    directory File.join(BUILDPATH, 'librhoimpl')
    file File.join(BUILDPATH, 'librhoimpl') => BUILDPATH

    directory File.join(BUILDPATH, 'rhoconnectclient')
    file File.join(BUILDPATH, 'rhoconnectclient') => BUILDPATH

    directory File.join(BUILDPATH, 'RhoImplJava')
    file File.join(BUILDPATH, 'RhoImplJava') => BUILDPATH

    def lib_objects(libfile)
      lib = File.basename(libfile).gsub(/\.a$/, "")

      sources = get_sources lib
      if USE_TRACES
        puts "#{lib} sources:"
        sources.each do |src|
            puts src
        end
      end

      objpath = File.join(BUILDPATH, lib)

      objects = get_objects sources, objpath
      objects
    end

    def obj_source(objfile)
      src = SRC.to_a().find { |src| (File.basename(src) + ".o") == File.basename(objfile) }
      if USE_TRACES
        puts "#{objfile} -----> #{src}"
      end
      src
    end

    file File.join(BUILDPATH, 'librhoconnectclient.so') => ["android:build:libraries"].concat(get_objects(get_sources('rhoconnectclient'),File.join(BUILDPATH, "rhoconnectclient"))) do |t|
      objects = t.prerequisites
      objects.delete_at(objects.index("android:build:libraries"))
      if USE_TRACES
        objects.each do |obj|
            puts "obj: #{obj}"
        end
      end
      args = ["-L#{BUILDPATH}"]

      args << "-Wl,--whole-archive"
      args << "-lrhoimpl"
      args << "-Wl,--no-whole-archive"

      libs = []
      LIBFN.each do |lib|
        libs << "-l" + File.basename(lib).gsub(/^lib/,"").gsub(/\.(a|so)$/,"")
      end
      libs << "-llog"
      libs << "-ldl"
      libs << "-lz"

      args += libs
      args += libs.reverse
      args += libs
      args += libs.reverse
      
      cc_link(t.name, objects, args) or exit 1
    end

    rule '.a' => [ lambda { |libfile| lib_objects(libfile) } ] do |t|
      lib = t.name.gsub(/.*\/lib/, "lib").gsub(/\.a$/, "")
      puts "#{lib}: #{t.name}" if USE_TRACES

      objpath = File.join(BUILDPATH, lib)
      sources = get_sources lib
      objects = get_objects sources, objpath

      cc_ar(t.name, objects) or exit 1
    end

    rule '.o' => [lambda { |objfile| obj_source(objfile) }, "android:config"] do |t|
      lib = $obj_lib[t.name]
      puts "#{lib}: building #{t.name} from #{t.source}. Args: #{BUILDARGS[lib]}" if USE_TRACES
      puts "pwd: #{Dir.pwd}" if USE_TRACES
      cc_compile(t.source, File.dirname(t.name), BUILDARGS[lib]) or exit 1
    end

    file File.join(BUILDPATH, "rhoimpl.jar") => [File.join(BUILDPATH, 'RhoImplJava')].concat(get_sources("rhoimpljava")) do |t|
      $androidjar = File.join($androidsdkpath, "platforms", $androidplatform, "android.jar")

      classpath = $androidjar
      classpath += $path_separator + BUILDPATH

      jar_build("rhoimpljava", classpath, t.name, File.join(BUILDPATH, 'RhoImplJava'))
    end

#    rule '.class' => [lambda { |classfile| class_source(classfile) }] do |t|
#    end


