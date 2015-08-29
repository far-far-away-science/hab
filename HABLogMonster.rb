#!/usr/bin/env ruby

require 'Date'

# Ruby 2.0 compliant

fail "Usage:\r\n./HABLogMonster.rb yyyymmdd-hhmmss.log [interval (s)]" unless ARGV.length > 0
log = ARGV[0]
log_prefix = log[0...(log.rindex('.'))] + '_'
# Compression interval in seconds
interval = (ARGV.length > 1) ? ARGV[1].to_i : 30
File.open(log, 'r') do |fh|
	# File.readlines would use too much memory on 100 MB+ log files
	open = {}
	until fh.eof?
		date, category, entry = fh.readline.strip.split(',', 3)
		# Much faster than DateTime::parse
		dp = DateTime.strptime(date, '%Y-%m-%d %H:%M:%S').to_time
		if not open.include? category
			# Open a new log if necessary
			newlog = log_prefix + category + '.log'
			ifh = File.open(newlog, 'w')
			open[category] = {:handle => ifh, :last_write => nil}
		end
		# Check for log aggregation
		of = open[category]
		if of[:last_write].nil? or dp - of[:last_write] >= interval
			# This may trash points at the end of the data... we will not care
			of[:handle].write(date + ',' + entry + "\n")
			of[:last_write] = dp
		end
	end
	# Close all open files
	open.each_value do |ifh|
		begin
			ifh[:handle].close
		rescue IOError
			# Allow all to close even if some have errors
		end
	end
end
