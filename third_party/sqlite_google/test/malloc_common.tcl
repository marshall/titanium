
# Usage: do_malloc_test <test number> <options...>
#
# The first argument, <test number>, is an integer used to name the
# tests executed by this proc. Options are as follows:
#
#     -tclprep          TCL script to run to prepare test.
#     -sqlprep          SQL script to run to prepare test.
#     -tclbody          TCL script to run with malloc failure simulation.
#     -sqlbody          TCL script to run with malloc failure simulation.
#     -cleanup          TCL script to run after the test.
#
# This command runs a series of tests to verify SQLite's ability
# to handle an out-of-memory condition gracefully. It is assumed
# that if this condition occurs a malloc() call will return a
# NULL pointer. Linux, for example, doesn't do that by default. See
# the "BUGS" section of malloc(3).
#
# Each iteration of a loop, the TCL commands in any argument passed
# to the -tclbody switch, followed by the SQL commands in any argument
# passed to the -sqlbody switch are executed. Each iteration the
# Nth call to sqliteMalloc() is made to fail, where N is increased
# each time the loop runs starting from 1. When all commands execute
# successfully, the loop ends.
#
proc do_malloc_test {tn args} {
  array unset ::mallocopts 
  array set ::mallocopts $args

  if {[string is integer $tn]} {
    set tn malloc-$tn
  }

  set ::go 1
  for {set ::n 1} {$::go && $::n < 50000} {incr ::n} {
    do_test $tn.$::n {

      # Remove all traces of database files test.db and test2.db from the files
      # system. Then open (empty database) "test.db" with the handle [db].
      # 
      sqlite_malloc_fail 0
      catch {db close} 
      catch {file delete -force test.db}
      catch {file delete -force test.db-journal}
      catch {file delete -force test2.db}
      catch {file delete -force test2.db-journal}
      catch {sqlite3 db test.db} 
      set ::DB [sqlite3_connection_pointer db]

      # Execute any -tclprep and -sqlprep scripts.
      #
      if {[info exists ::mallocopts(-tclprep)]} {
        eval $::mallocopts(-tclprep)
      }
      if {[info exists ::mallocopts(-sqlprep)]} {
        execsql $::mallocopts(-sqlprep)
      }

      # Now set the ${::n}th malloc() to fail and execute the -tclbody and
      # -sqlbody scripts.
      #
      sqlite_malloc_fail $::n
      set ::mallocbody {}
      if {[info exists ::mallocopts(-tclbody)]} {
        append ::mallocbody "$::mallocopts(-tclbody)\n"
      }
      if {[info exists ::mallocopts(-sqlbody)]} {
        append ::mallocbody "db eval {$::mallocopts(-sqlbody)}"
      }
      set v [catch $::mallocbody msg]

      # If the test fails (if $v!=0) and the database connection actually
      # exists, make sure the failure code is SQLITE_NOMEM.
      if {$v && [info command db]=="db" && [info exists ::mallocopts(-sqlbody)]
              && [db errorcode]!=7} {
        set v 999
      }

      set leftover [lindex [sqlite_malloc_stat] 2]
      if {$leftover>0} {
        if {$leftover>1} {puts "\nLeftover: $leftover\nReturn=$v  Message=$msg"}
        set ::go 0
        if {$v} {
          puts "\nError message returned: $msg"
        } else {
          set v {1 1}
        }
      } else {
        set v2 [expr {$msg=="" || $msg=="out of memory"}]
        if {!$v2} {puts "\nError message returned: $msg"}
        lappend v $v2
      }
    } {1 1}

    if {[info exists ::mallocopts(-cleanup)]} {
      catch [list uplevel #0 $::mallocopts(-cleanup)] msg
    }
  }
  unset ::mallocopts
}

