## This should be a script with configuration args, for now run it via pasting...

/usr/local/R-4.5.3/bin/R --no-readline

suppressMessages(library(dplyr))
suppressMessages(library(readr))
suppressMessages(library(tidyr))
suppressMessages(library(stringr))
suppressMessages(library(forcats))
suppressMessages(library(ggplot2))

x11(type="cairo") # On Linux

theme_jm1 <- theme_bw() + # A decent theme for HTML output
    theme(
        plot.title  = element_text(size = rel(1.5), family = 'Helvetica', face = 'bold'),
        plot.subtitle  = element_text(size = rel(1.3), family = 'Helvetica', face = 'bold'),
        axis.title  = element_text(size = rel(1.5), colour = 'black', face = 'bold'),
        axis.text.x = element_text(angle=0, size = rel(1.5), lineheight = 0.9, colour = 'black', vjust = 1, face = 'bold'),
        axis.text.y = element_text(size = rel(1.5), lineheight = 0.9, colour = 'black', hjust = 1, face = 'bold'),
        strip.text = element_text(size = rel(1.6)),
        legend.title = element_text(size = rel(1.75)),
        legend.key  = element_rect(colour = 'white', fill = 'white'),
        legend.text = element_text(size = rel(1.3))
    )

gen_thread_summary <- function(threads, p_fun_name) { # Thread summary for 'date time' timestamps
    threads <- threads %>% mutate(utime = as.numeric(as.POSIXct(str_c(date,' ',time))),
                                  rtime = utime - min(utime)) %>% select(-date, -time, -utime) %>% arrange(rtime)
    threads <- threads %>% mutate(rtime = 1000 * rtime) # Make it milliseconds
    udp_overhead_time <- max(threads %>% filter(func == 'prep') %>% pull(rtime)) - min(threads %>% filter(func == 'prep') %>% pull(rtime))

    first_thread <- min(threads %>% filter(func == p_fun_name) %>% pull(rtime))- min(threads %>% filter(func == 'process') %>% pull(rtime))
        
    thread_summary <- threads %>% pivot_wider(names_from = state, values_from = rtime) %>% arrange(Start) %>% mutate(dt = End - Start)
    thread_summary <- thread_summary %>% arrange(func, Start)
    thread_summary <- thread_summary %>% filter(func != 'process') %>% select(func, Start, End) %>%
        mutate(func = if_else(func == p_fun_name, str_c(func, '_', row_number()), func))
    thread_summary <- thread_summary %>% arrange(Start) %>% mutate(dt = End - Start)

    ## Threads may not start, or be reported, in order
    func_order          <- thread_summary %>% pull(func) # Retain actual thread start order
    thread_summary$func <- fct_relevel(thread_summary$func, func_order)

    return(list(udp_overhead_time = udp_overhead_time, first_thread = first_thread, thread_summary = thread_summary))
}

plot_r_threads <- function(thread_sum, title) {
    plot_obj <- ggplot(thread_sum, aes(y = func)) +
        geom_segment(aes(x = Start, xend = End, yend = func), linewidth = 4, color = 'blue') +
        labs(x = 'Time, ms', y = '') +
        scale_x_continuous(breaks = seq(0, max(thread_sum$End), 10), minor_breaks = seq(0, max(thread_sum$End), 5)) +
        ggtitle(title) +
        theme_jm1
    return(plot_obj)
}

test_pipe <- 'nc -lku 1800'
con <- pipe(test_pipe, 'r')

fun_name = 'cca_etp'
fun_name = 'future_example'

while(TRUE) {
    raw_data <- NULL
    while(TRUE) {
        line_in <- readLines(con, n = 1)
        print(line_in)
        if (str_detect(line_in, 'Done')) break
        raw_data <- c(raw_data, line_in)
    }

    raw_data
    threads1 <- read_table(raw_data, col_types = 'cccnc')
    threads1
    threads_summary <- gen_thread_summary(threads1, p_fun_name = fun_name)
    print(threads_summary)
    ## plot_r_threads(threads_summary, 'process-udp.R') # Not working?
}

close(con)

