class StructuredTextUtils < Formula
  desc "A group of utilities to deal with structured text such as json or yaml."
  homepage "https://github.com/programmiersportgruppe/structured-text-utils"
  url "http://www.programmiersportgruppe.org/files/2013/02/structured-text-utils-0.1-Darwin-x86_64.tar_.bz2"
  sha256 "3467e8ae2b1d02bf73e7fdeb5eba3ac7ab2070d179d3268d110014c4363db004"
  # depends_on "cmake" => :build

  def install
    # ENV.deparallelize  # if your formula fails when building in parallel
    # Remove unrecognized options if warned by configure
    # system "./configure", "--disable-debug",
    #                       "--disable-dependency-tracking",
    #                       "--disable-silent-rules",
    #                       "--prefix=#{prefix}"
    # system "cmake", ".", *std_cmake_args
    # system "make", "install" # if this fails, try separate make/make install steps
    system "tar xvf structured-text-utils-0.1-Darwin-x86_64.tar_ --strip-components=1"
    # cd "structured-text-utils-0.1-Darwin-x86_64"
    inreplace "./install.sh" do |s|
        s.gsub! "/usr/local", "#{prefix}"
    end
    system "./install.sh"
  end

  test do
    # `test do` will create, run in and delete a temporary directory.
    #
    # This test will fail and we won't accept that! For Homebrew/homebrew-core
    # this will need to be a test that verifies the functionality of the
    # software. Run the test with `brew test structured-text-utils`. Options passed
    # to `brew install` such as `--HEAD` also need to be provided to `brew test`.
    #
    # The installed folder is not in the path, so use the entire path to any
    # executables being tested: `system "#{bin}/program", "do", "something"`.
    system "false"
  end
end
