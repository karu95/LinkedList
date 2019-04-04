# LinkedList performance evaluation.

### Serial linked list

```
./serial_linked_list list_size sample_operations member_op_percentage insert_op_percentage delete_op_percentage thread_count
```

### Parallel linked list using mutexes

```
./parallel_linked_list_rwlocks list_size sample_operations member_op_percentage insert_op_percentage delete_op_percentage thread_count
```

### Parallel linked list using RW locks

```
./parallel_linked_list_mutex list_size sample_operations member_op_percentage insert_op_percentage delete_op_percentage thread_count
```
