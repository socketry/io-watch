def build
	ext_path = File.expand_path("ext", context.root)
	system("./configure", chdir: ext_path)
	system("make", "install", chdir: ext_path)
end